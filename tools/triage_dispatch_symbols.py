"""Rank verified command and screen targets by exported decompile size."""

import csv
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
FUNCTIONS = {}
with (ROOT / "architecture/function_map.jsonl").open(encoding="utf-8") as handle:
    for line in handle:
        record = json.loads(line)
        FUNCTIONS[record["address"].lower()] = record


def report(csv_name, address_column, name_column):
    with (ROOT / "architecture" / csv_name).open(newline="", encoding="utf-8") as handle:
        entries = list(csv.DictReader(handle))
    addresses = {entry[address_column].lower() for entry in entries}
    print(f"{csv_name}: {len(entries)} entries, {len(addresses)} distinct targets")
    rows = []
    for address in addresses:
        info = FUNCTIONS.get(address)
        rows.append((info.get("decompiled_chars", 10**9) if info else 10**9,
                     address,
                     [entry[name_column] for entry in entries
                      if entry[address_column].lower() == address],
                     len(info.get("callees", [])) if info else -1))
    for chars, address, names, callees in sorted(rows):
        print(f"  {address} chars={chars:5} calls={callees:2} {', '.join(names)}")


report("presentation_commands.csv", "handler_address", "command")
report("screen_task_table.csv", "activate_address", "screen")
