import json
from pathlib import Path

state = json.loads(Path("reports/pipeline/state.json").read_text(encoding="utf-8"))
integrated = set(a.lower() for a, e in state.get("functions", {}).items() if e.get("integration") == "integrated")

# Focus: plage projection 0x006d0000-0x006e0000
lo, hi = 0x006d0000, 0x006e0000

candidates = []
for p in Path(".ghidra-exports").glob("*.json"):
    try:
        addr = int(p.stem, 16)
    except ValueError:
        continue
    addr_str = f"{addr:08x}"
    if addr_str in integrated:
        continue
    if lo <= addr < hi:
        candidates.append(addr_str)

candidates.sort()
print(f"{len(candidates)} candidats non integrates dans 0x006d0000-0x006e0000")
for a in candidates:
    print(f"  {a}")
