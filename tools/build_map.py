"""Create a compact, evidence-linked call and data map from Ghidra exports."""
from __future__ import annotations

import json
import re
from collections import Counter
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / ".ghidra-exports"
OUT = ROOT / "architecture"


def read(name: str):
    return json.loads((SOURCE / name).read_text(encoding="utf-8-sig"))


def main() -> None:
    OUT.mkdir(exist_ok=True)
    index = read("_index.json")
    global_refs = read("_global_refs_by_function.json")
    string_refs = read("_string_refs_by_function.json")
    globals_by_address = read("_globals.json")
    structures = read("_structs.json")
    vtables = read("_vtables.json")
    components = Counter()
    edges = []
    with (OUT / "function_map.jsonl").open("w", encoding="utf-8") as stream:
        for address in sorted(index):
            export = read(address + ".json")
            calls = sorted({c["addr"].lower() for c in export.get("callees", [])
                            if c.get("addr", "").lower() in index})
            edges.extend((address, callee) for callee in calls)
            record = {
                "address": address, "name": export.get("name"),
                "signature": export.get("signature"),
                "calling_convention": export.get("calling_convention"),
                "is_thunk": export.get("is_thunk", False),
                "decompiled_chars": len(export.get("decompiled", "")),
                "unresolved_registers": bool(re.search(
                    r"\b(?:unaff_|extraout_|in_[A-Z])", export.get("decompiled", ""))),
                "callees": calls,
                "external_callees": [c for c in export.get("callees", [])
                                     if c.get("addr", "").lower() not in index],
                "globals": sorted(set(global_refs.get(address, []))),
                "strings": sorted(set(string_refs.get(address, []))),
                "export": f".ghidra-exports/{address}.json",
            }
            stream.write(json.dumps(record, ensure_ascii=False) + "\n")
            components["functions"] += 1
    with (OUT / "call_edges.tsv").open("w", encoding="utf-8") as stream:
        stream.write("caller\tcallee\n")
        for source, target in edges:
            stream.write(f"{source}\t{target}\n")
    summary = {
        "function_count": components["functions"],
        "call_edges": len(edges), "global_count": len(globals_by_address),
        "structure_count": len(structures), "vtable_count": len(vtables),
        "global_references": sum(map(len, global_refs.values())),
        "string_references": sum(map(len, string_refs.values())),
        "source": ".ghidra-exports",
        "caveat": "Names, signatures, and references are Ghidra evidence, not verified source types.",
    }
    (OUT / "dependency_summary.json").write_text(
        json.dumps(summary, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(summary, indent=2))


if __name__ == "__main__":
    main()
