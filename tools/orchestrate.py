"""Bounded, resumable ReAgent batches for the HP6 reconstruction.

ReAgent owns the initial reconstruction. This script records its output as a
candidate; integration and equivalence remain separate review stages.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
EXPORTS = ROOT / ".ghidra-exports"
REPORTS = ROOT / "reports" / "pipeline"
STATE = REPORTS / "state.json"
REAGENT_PROGRESS = ROOT / "re-agent-progress.json"
REAGENT = ROOT / ".venv" / "Scripts" / "re-agent.exe"


def save_json(path: Path, data: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    temporary.replace(path)


def load_json(path: Path, default: object) -> object:
    return json.loads(path.read_text(encoding="utf-8-sig")) if path.exists() else default


def run(args: list[str], log: Path) -> int:
    with log.open("w", encoding="utf-8") as output:
        process = subprocess.run(args, cwd=ROOT, stdout=output, stderr=subprocess.STDOUT,
                                 check=False)
    return process.returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--batch", required=True, help="Stable batch identifier")
    parser.add_argument("--address", action="append", required=True, help="Hex seed; repeat 5–10 times")
    parser.add_argument("--max-rounds", type=int, default=1)
    parser.add_argument("--plan-only", action="store_true")
    parser.add_argument("--audit-integrated", action="store_true",
                        help="Obtain missing ReAgent candidates for integrated functions without changing integration")
    args = parser.parse_args()
    addresses = sorted(set(a.lower().removeprefix("0x").zfill(8) for a in args.address))
    if not 5 <= len(addresses) <= 10:
        parser.error("Each batch must contain 5 to 10 distinct functions")
    for address in addresses:
        if not (EXPORTS / f"{address}.json").is_file():
            parser.error(f"Missing Ghidra export: {address}")

    REPORTS.mkdir(parents=True, exist_ok=True)
    state = load_json(STATE, {"schema_version": 1, "batches": {}, "functions": {}})
    batch = state["batches"].setdefault(args.batch, {"addresses": addresses})
    if set(batch["addresses"]) != set(addresses):
        parser.error("Batch identifier already belongs to different addresses")
    manifest = REPORTS / f"{args.batch}.manifest.json"
    batch["manifest"] = str(manifest.relative_to(ROOT))
    batch["updated_at"] = datetime.now(timezone.utc).isoformat()
    save_json(STATE, state)

    if not manifest.exists():
        command = [str(REAGENT), "plan"]
        for address in addresses:
            command += ["--address", address]
        command += ["--max-depth", "0", "--max-functions", str(len(addresses)),
                    "--output", str(manifest)]
        code = run(command, REPORTS / f"{args.batch}.plan.log")
        if code and not manifest.exists():
            state = load_json(STATE, state)
            batch = state["batches"][args.batch]
            batch["status"] = "plan_failed"
            save_json(STATE, state)
            return code
    planned = load_json(manifest, {})
    actual = {item["address"].lower() for item in planned.get("functions", [])}
    state = load_json(STATE, state)
    unresolved_gaps = []
    for gap in planned.get("gaps", []):
        match = re.fullmatch(r"Dependency ([0-9a-fA-F]+) outside bounded selection",
                             gap.get("reason", ""))
        dependency = match.group(1).lower() if match else ""
        if state["functions"].get(dependency, {}).get("integration") != "integrated":
            unresolved_gaps.append(gap)
    if actual != set(addresses) or unresolved_gaps:
        state = load_json(STATE, state)
        batch = state["batches"][args.batch]
        batch["status"] = "plan_incomplete"
        batch["unresolved_gaps"] = unresolved_gaps
        save_json(STATE, state)
        return 2
    state = load_json(STATE, state)
    batch = state["batches"][args.batch]
    batch["resolved_external_dependencies"] = [
        gap["reason"] for gap in planned.get("gaps", [])]
    batch["status"] = "planned"
    save_json(STATE, state)
    if args.plan_only:
        return 0

    # One function per invocation gives a durable checkpoint if a long run stops.
    for item in planned["functions"]:
        address = item["address"].lower()
        state = load_json(STATE, state)
        entry = state["functions"].setdefault(address, {})
        entry.update({"batch": args.batch, "export": f".ghidra-exports/{address}.json",
                      "ghidra_signature": planned["evidence"][address]["context"]["function"].get("signature")})
        # An integrated function may have come from a rejected ReAgent
        # candidate reviewed against x86. Do not overwrite that decision.
        reviewed = entry.get("integration") in {"integrated", "deferred"}
        if entry.get("candidate_sha256") or (reviewed and not args.audit_integrated):
            continue
        log = REPORTS / f"{args.batch}.{address}.reverse.log"
        code = run([str(REAGENT), "reverse", "--address", address,
                    "--max-rounds", str(args.max_rounds)], log)
        progress = load_json(REAGENT_PROGRESS, {})
        result = progress.get("functions", {}).get(address, {})
        candidate = result.get("code") or ""
        state = load_json(STATE, state)
        entry = state["functions"].setdefault(address, entry)
        entry.update({"reagent_exit_code": code, "reagent_verdict": result.get("verdict"),
                      "reagent_validation": result.get("validation_verdict"),
                      "reagent_run_id": result.get("run_id"), "error": result.get("error"),
                      "updated_at": datetime.now(timezone.utc).isoformat()})
        if code == 0 and candidate and result.get("success") and result.get("run_id"):
            path = REPORTS / "candidates" / f"{address}.cpp"
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(candidate + "\n", encoding="utf-8")
            entry.update({"candidate": str(path.relative_to(ROOT)),
                          "candidate_sha256": hashlib.sha256(candidate.encode()).hexdigest()})
            if reviewed:
                entry["reagent_audit"] = "pending_review"
            else:
                entry.update({"integration": "pending", "build": "not_run",
                              "equivalence": "unverified"})
        else:
            if reviewed:
                entry["reagent_audit"] = "blocked"
            else:
                entry["integration"] = "blocked"
            if candidate:
                path = REPORTS / "rejected" / f"{address}.cpp"
                path.parent.mkdir(parents=True, exist_ok=True)
                path.write_text(candidate + "\n", encoding="utf-8")
                entry["rejected_candidate"] = str(path.relative_to(ROOT))
                entry["rejected_sha256"] = hashlib.sha256(candidate.encode()).hexdigest()
        save_json(STATE, state)
        print(f"{address}: {entry['integration']} ({entry.get('reagent_verdict')})", flush=True)
        error_text = str(result.get("error") or "")
        if "usage limit" in error_text.lower() or "try again at" in error_text.lower():
            state["batches"][args.batch]["status"] = "llm_usage_limited"
            save_json(STATE, state)
            return code or 3
    state = load_json(STATE, state)
    batch = state["batches"][args.batch]
    batch["status"] = "candidates_recorded" if all(
        state["functions"].get(address, {}).get("candidate_sha256") or
        (not args.audit_integrated and
         state["functions"].get(address, {}).get("integration") in {"integrated", "deferred"})
        for address in addresses
    ) else "partial"
    save_json(STATE, state)
    return 0


if __name__ == "__main__":
    sys.exit(main())
