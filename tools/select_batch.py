"""Suggest a bounded dependency-coherent ReAgent batch from the Ghidra map.

Heuristics rank candidates; they do not infer classes or certify behavior.
"""
from __future__ import annotations

import argparse
import json
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MAP = ROOT / "architecture" / "function_map.jsonl"
STATE = ROOT / "reports" / "pipeline" / "state.json"


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--size", type=int, default=5)
    parser.add_argument("--anchor", help="Prefer functions calling this address")
    args = parser.parse_args()
    if not 5 <= args.size <= 10:
        parser.error("size must be 5–10")
    records = {item["address"]: item for item in (
        json.loads(line) for line in MAP.read_text(encoding="utf-8").splitlines())}
    state_functions = json.loads(STATE.read_text(encoding="utf-8")).get("functions", {})
    tracked = set(state_functions)
    completed = {address for address, entry in state_functions.items()
                 if entry.get("integration") == "integrated"}
    eligible = {address: item for address, item in records.items()
                if address not in tracked and not item["is_thunk"]
                and item["name"].startswith("FUN_")
                and 60 <= item["decompiled_chars"] <= 1100
                and not item["unresolved_registers"]
                and len(item["callees"]) <= 2 and len(item["globals"]) <= 4}
    groups = defaultdict(list)
    for address, item in eligible.items():
        for callee in item["callees"]:
            groups[callee].append(address)

    def closure(address: str, visiting: set[str] | None = None) -> set[str] | None:
        """All non-integrated dependencies needed for one function."""
        if address in completed:
            return set()
        if address not in eligible:
            return None
        if visiting is None:
            visiting = set()
        if address in visiting:
            return set()
        visiting = visiting | {address}
        result = {address}
        for dependency in eligible[address]["callees"]:
            nested = closure(dependency, visiting)
            if nested is None:
                return None
            result.update(nested)
        return result
    anchor = args.anchor.lower().removeprefix("0x") if args.anchor else None
    candidates = []
    for callee, members in groups.items():
        if len(members) < args.size or len(members) > 40:
            continue
        # Include the shared callee and all other unresolved dependencies.
        # A batch proposal must already be closed under Ghidra call edges.
        ranked = sorted(members, key=lambda a: (eligible[a]["decompiled_chars"], a))
        selected: set[str] = set()
        represented = 0
        for member in ranked:
            required = closure(member)
            if required is None or len(selected | required) > args.size:
                continue
            selected.update(required)
            represented += 1
            if len(selected) == args.size:
                break
        if len(selected) != args.size or represented < 2:
            continue
        selected_list = sorted(selected)
        mean_size = sum(eligible[a]["decompiled_chars"] for a in selected_list) / args.size
        score = (0 if callee == anchor else 1, mean_size, -len(members), callee)
        candidates.append((score, callee, selected_list, len(members)))
    if not candidates:
        parser.error("No eligible shared-callee group; choose addresses manually")
    _, callee, selected, pool = min(candidates)
    output = {"shared_callee": callee, "eligible_pool": pool,
              "addresses": selected,
              "evidence": "At least two selected functions share this callee; all unresolved Ghidra callees are included in the bounded batch; no class inferred."}
    print(json.dumps(output, indent=2))


if __name__ == "__main__":
    main()
