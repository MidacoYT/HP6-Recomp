"""Record reviewed integration decisions without changing ReAgent history."""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
STATE = ROOT / "reports" / "pipeline" / "state.json"
MAPPING = ROOT / "architecture" / "address_map.csv"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--address", required=True)
    parser.add_argument("--integration", choices=["integrated", "deferred", "blocked"], required=True)
    parser.add_argument("--symbol", default="")
    parser.add_argument("--source", default="")
    parser.add_argument("--build", choices=["passed", "compiled_only", "not_run", "failed"], default="not_run")
    parser.add_argument("--note", default="")
    args = parser.parse_args()
    address = args.address.lower().removeprefix("0x").zfill(8)
    data = json.loads(STATE.read_text(encoding="utf-8"))
    if address not in data["functions"]:
        parser.error(f"No ReAgent record for {address}")
    if args.integration == "integrated" and (not args.symbol or not args.source):
        parser.error("Integrated functions need a symbol and source path")
    record = data["functions"][address]
    record.update({"integration": args.integration, "cpp_symbol": args.symbol,
                   "source": args.source, "build": args.build,
                   "integration_note": args.note, "equivalence": "unverified"})
    temporary = STATE.with_suffix(".json.tmp")
    temporary.write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    temporary.replace(STATE)
    MAPPING.parent.mkdir(exist_ok=True)
    with MAPPING.open("w", encoding="utf-8", newline="") as stream:
        writer = csv.writer(stream)
        writer.writerow(["address", "ghidra_signature", "cpp_symbol", "source", "integration", "build", "equivalence"])
        for key, item in sorted(data["functions"].items()):
            writer.writerow([key, item.get("ghidra_signature"), item.get("cpp_symbol"),
                             item.get("source"), item.get("integration"), item.get("build"),
                             item.get("equivalence")])


if __name__ == "__main__":
    main()
