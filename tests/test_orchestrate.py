"""Safety checks for resumable ReAgent state transitions."""
from __future__ import annotations

import json
import sys
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from tools import orchestrate


ADDRESSES = [f"006b2d{i:02x}" for i in range(5)]


class AuditIntegratedTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.reports = self.root / "reports" / "pipeline"
        self.reports.mkdir(parents=True)
        self.exports = self.root / ".ghidra-exports"
        self.exports.mkdir()
        for address in ADDRESSES:
            (self.exports / f"{address}.json").write_text("{}", encoding="utf-8")
        manifest = {
            "functions": [{"address": address} for address in ADDRESSES],
            "gaps": [],
            "evidence": {address: {"context": {"function": {"signature": "void f()"}}}
                         for address in ADDRESSES},
        }
        (self.reports / "audit.manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        self.state = self.reports / "state.json"
        initial = {
            "schema_version": 1,
            "batches": {},
            "functions": {address: {"integration": "integrated", "build": "passed",
                                    "equivalence": "unverified", "source": "src/runtime/Pool.cpp"}
                          for address in ADDRESSES},
        }
        self.state.write_text(json.dumps(initial), encoding="utf-8")
        self.progress = self.root / "re-agent-progress.json"
        self.patches = [
            patch.object(orchestrate, "ROOT", self.root),
            patch.object(orchestrate, "REPORTS", self.reports),
            patch.object(orchestrate, "STATE", self.state),
            patch.object(orchestrate, "EXPORTS", self.exports),
            patch.object(orchestrate, "REAGENT_PROGRESS", self.progress),
            patch.object(sys, "argv", ["orchestrate.py", "--batch", "audit",
                                       "--audit-integrated", *sum(
                                           (["--address", address] for address in ADDRESSES), [])]),
        ]
        for item in self.patches:
            item.start()
            self.addCleanup(item.stop)

    def test_success_keeps_integration_and_records_review(self) -> None:
        def run(_command: list[str], _log: Path) -> int:
            address = _command[_command.index("--address") + 1]
            self.progress.write_text(json.dumps({"functions": {address: {
                "run_id": f"run-{address}", "success": True,
                "verdict": "PASS", "code": f"void f_{address}() {{}}",
            }}}), encoding="utf-8")
            return 0

        with patch.object(orchestrate, "run", side_effect=run):
            self.assertEqual(orchestrate.main(), 0)
        result = json.loads(self.state.read_text(encoding="utf-8"))
        for address in ADDRESSES:
            entry = result["functions"][address]
            self.assertEqual(entry["integration"], "integrated")
            self.assertEqual(entry["build"], "passed")
            self.assertEqual(entry["reagent_audit"], "pending_review")
            self.assertTrue(entry["candidate_sha256"])

    def test_usage_limit_stops_batch_and_preserves_integration(self) -> None:
        def run(_command: list[str], _log: Path) -> int:
            self.progress.write_text(json.dumps({"functions": {ADDRESSES[0]: {
                "run_id": "", "success": False, "code": None,
                "error": "You've hit your usage limit. Try again at 01:19 UTC",
            }}}), encoding="utf-8")
            return 1

        with patch.object(orchestrate, "run", side_effect=run) as called:
            self.assertEqual(orchestrate.main(), 1)
            self.assertEqual(called.call_count, 1)
        result = json.loads(self.state.read_text(encoding="utf-8"))
        self.assertEqual(result["batches"]["audit"]["status"], "llm_usage_limited")
        entry = result["functions"][ADDRESSES[0]]
        self.assertEqual(entry["integration"], "integrated")
        self.assertEqual(entry["build"], "passed")
        self.assertEqual(entry["reagent_audit"], "blocked")

    def test_audit_does_not_reopen_deferred_decision(self) -> None:
        state = json.loads(self.state.read_text(encoding="utf-8"))
        state["functions"][ADDRESSES[0]]["integration"] = "deferred"
        state["functions"][ADDRESSES[0]]["build"] = "not_run"
        self.state.write_text(json.dumps(state), encoding="utf-8")

        def run(_command: list[str], _log: Path) -> int:
            address = _command[_command.index("--address") + 1]
            self.progress.write_text(json.dumps({"functions": {address: {
                "run_id": f"run-{address}", "success": True,
                "verdict": "PASS", "code": f"void f_{address}() {{}}",
            }}}), encoding="utf-8")
            return 0

        with patch.object(orchestrate, "run", side_effect=run):
            self.assertEqual(orchestrate.main(), 0)
        result = json.loads(self.state.read_text(encoding="utf-8"))
        entry = result["functions"][ADDRESSES[0]]
        self.assertEqual(entry["integration"], "deferred")
        self.assertEqual(entry["build"], "not_run")
        self.assertEqual(entry["reagent_audit"], "pending_review")


if __name__ == "__main__":
    unittest.main()
