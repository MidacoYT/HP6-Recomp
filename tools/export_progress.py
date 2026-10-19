"""Export reports/pipeline/state.json into a small JSON suited for the
static progress site in docs/ (docs/data/progress.json).

Read-only with respect to the ReAgent pipeline state; does not change any
integration decision, only reshapes it for display.
"""
from __future__ import annotations

import json
import re
from datetime import datetime, timezone
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATE = ROOT / "reports" / "pipeline" / "state.json"
OUTPUT = ROOT / "docs" / "data" / "progress.json"

MODULE_PATTERN = re.compile(r"^(src/[^/]+)/")


def module_for(entry: dict) -> str:
    source = entry.get("source") or ""
    match = MODULE_PATTERN.match(source)
    if match:
        return match.group(1)
    if source == "records/crt" or "CRT external" in (entry.get("cpp_symbol") or ""):
        return "CRT externe"
    if "STL internal" in (entry.get("cpp_symbol") or ""):
        return "STL externe"
    if entry.get("integration") == "integrated":
        return "autre"
    return "non assigne"


def main() -> None:
    data = json.loads(STATE.read_text(encoding="utf-8"))
    functions: dict[str, dict] = data["functions"]

    totals = {"total": 0, "integrated": 0, "deferred": 0, "blocked": 0}
    module_totals: dict[str, dict[str, int]] = {}
    rows: list[dict] = []

    for address, entry in functions.items():
        status = entry.get("integration") or "pending"
        totals["total"] += 1
        totals[status] = totals.get(status, 0) + 1

        module = module_for(entry)
        bucket = module_totals.setdefault(module, {"total": 0, "integrated": 0})
        bucket["total"] += 1
        if status == "integrated":
            bucket["integrated"] += 1

        rows.append({
            "address": address,
            "symbol": entry.get("cpp_symbol") or "",
            "source": entry.get("source") or "",
            "module": module,
            "status": status,
            "build": entry.get("build") or "not_run",
            "batch": entry.get("batch") or "",
            "note": entry.get("integration_note") or "",
        })

    rows.sort(key=lambda row: row["address"])

    modules = [
        {
            "name": name,
            "total": counts["total"],
            "integrated": counts["integrated"],
            "percent": round(100.0 * counts["integrated"] / counts["total"], 1) if counts["total"] else 0.0,
        }
        for name, counts in module_totals.items()
    ]
    modules.sort(key=lambda m: (-m["total"], m["name"]))

    percent = round(100.0 * totals["integrated"] / totals["total"], 1) if totals["total"] else 0.0

    payload = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "project": {
            "name": "Harry Potter and the Half-Blood Prince (PC)",
            "slug": "HP6-Recomp",
            "repo": "https://github.com/MidacoYT/HP6-Recomp",
        },
        "totals": {**totals, "percent": percent},
        "modules": modules,
        "functions": rows,
    }

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(payload, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Wrote {OUTPUT} ({totals['total']} functions, {percent}% integrees)")


if __name__ == "__main__":
    main()
