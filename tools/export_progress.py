"""Export reports/pipeline/state.json into a small JSON suited for the
static progress site in docs/ (docs/data/progress.json).

Also cross-references the independent IDA knowledge base's full function
catalogue (../knowledge/hp6/functions.json, a sibling of this repo, not
checked in here) so the site can show a percentage against the whole
game's ~14k functions, not just the subset this project has reviewed so
far. If that sibling catalogue isn't found (e.g. a clean clone without
the neighbour project), the script falls back to reviewed-subset-only
output, matching the script's older behaviour.

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
IDA_CATALOGUE = ROOT.parent / "knowledge" / "hp6" / "functions.json"

MODULE_PATTERN = re.compile(r"^(src/[^/]+)/")

# Short keys for the (potentially ~14k-row) function list, to keep the
# JSON payload reasonably small for mobile connections. Decoded in app.js.
STATUS_CODE = {"integrated": "i", "deferred": "d", "blocked": "b", "untouched": "u"}
BUILD_CODE = {"passed": "p", "compiled_only": "c", "not_run": "n", "failed": "f"}


def normalize_address(value: str) -> str:
    value = value.lower()
    if value.startswith("0x"):
        value = value[2:]
    return value.zfill(8)


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


def load_ida_catalogue() -> dict[str, dict]:
    if not IDA_CATALOGUE.exists():
        return {}
    raw = json.loads(IDA_CATALOGUE.read_text(encoding="utf-8"))
    catalogue: dict[str, dict] = {}
    for entry in raw:
        address = normalize_address(entry["address"])
        catalogue[address] = {
            "name": entry.get("name") or "",
            "automatic_name": bool(entry.get("automatic_name")),
        }
    return catalogue


def main() -> None:
    data = json.loads(STATE.read_text(encoding="utf-8"))
    functions: dict[str, dict] = data["functions"]
    ida_catalogue = load_ida_catalogue()

    totals = {"total": 0, "integrated": 0, "deferred": 0, "blocked": 0}
    module_totals: dict[str, dict[str, int]] = {}
    rows: list[dict] = []
    seen_addresses: set[str] = set()

    for address, entry in functions.items():
        address = normalize_address(address)
        seen_addresses.add(address)
        status = entry.get("integration") or "pending"
        totals["total"] += 1
        totals[status] = totals.get(status, 0) + 1

        module = module_for(entry)
        bucket = module_totals.setdefault(module, {"total": 0, "integrated": 0})
        bucket["total"] += 1
        if status == "integrated":
            bucket["integrated"] += 1

        symbol = entry.get("cpp_symbol") or ""
        ida_name = ida_catalogue.get(address, {})
        name = symbol or ida_name.get("name") or ""

        rows.append({
            "a": address,
            "n": name,
            "auto": bool(ida_name) and not symbol and ida_name.get("automatic_name", False),
            "s": STATUS_CODE.get(status, "u"),
            "m": module if status != "untouched" else "",
            "b": BUILD_CODE.get(entry.get("build") or "not_run", "n"),
            "note": entry.get("integration_note") or "",
            "src": entry.get("source") or "",
        })

    reviewed_totals = dict(totals)

    # Merge in every function from the independent IDA catalogue that this
    # project hasn't reviewed yet, so the site can report a true percentage
    # against the whole game instead of only the reviewed subset.
    global_total = len(seen_addresses | set(ida_catalogue.keys()))
    for address, ida_entry in ida_catalogue.items():
        if address in seen_addresses:
            continue
        rows.append({
            "a": address,
            "n": ida_entry.get("name") or "",
            "auto": ida_entry.get("automatic_name", False),
            "s": "u",
            "m": "",
            "b": "n",
            "note": "",
            "src": "",
        })

    rows.sort(key=lambda row: row["a"])

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

    reviewed_percent = (
        round(100.0 * reviewed_totals["integrated"] / reviewed_totals["total"], 1)
        if reviewed_totals["total"] else 0.0
    )
    global_integrated = reviewed_totals["integrated"]
    global_deferred = reviewed_totals["deferred"]
    global_blocked = reviewed_totals.get("blocked", 0)
    global_untouched = global_total - global_integrated - global_deferred - global_blocked
    global_percent = round(100.0 * global_integrated / global_total, 2) if global_total else 0.0

    payload = {
        "generated_at": datetime.now(timezone.utc).isoformat(),
        "project": {
            "name": "Harry Potter and the Half-Blood Prince (PC)",
            "slug": "HP6-Recomp",
            "repo": "https://github.com/MidacoYT/HP6-Recomp",
        },
        "global": {
            "total": global_total,
            "integrated": global_integrated,
            "deferred": global_deferred,
            "blocked": global_blocked,
            "untouched": global_untouched,
            "percent": global_percent,
            "source": "IDA + Ghidra, union des deux catalogues" if ida_catalogue else "Ghidra uniquement (catalogue IDA introuvable)",
        },
        "reviewed": {**reviewed_totals, "percent": reviewed_percent},
        "modules": modules,
        "functions": rows,
    }

    OUTPUT.parent.mkdir(parents=True, exist_ok=True)
    OUTPUT.write_text(json.dumps(payload, ensure_ascii=False, separators=(",", ":")) + "\n", encoding="utf-8")
    print(
        f"Wrote {OUTPUT}: {global_total} fonctions au total "
        f"({global_integrated} integrees, {global_percent}%), "
        f"{reviewed_totals['total']} deja etudiees"
    )


if __name__ == "__main__":
    main()
