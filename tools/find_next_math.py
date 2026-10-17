import json
from pathlib import Path

state = json.loads(Path("reports/pipeline/state.json").read_text(encoding="utf-8"))
integrated = set(a.lower() for a, e in state.get("functions", {}).items() if e.get("integration") == "integrated")

# Zone math : 0x00400000 - 0x00600000
lo, hi = 0x00400000, 0x00600000

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
        d = json.loads(p.read_text(encoding="utf-8"))
        sig = d.get("signature", "")
        calls = d.get("calls", [])
        # Prioriser les fonctions float/math (pas de calls externes complexes)
        candidates.append((addr_str, sig, calls))

# Trier par adresse
candidates.sort()
print(f"{len(candidates)} candidats non integrates dans 0x00400000-0x00600000")
for addr, sig, calls in candidates[:40]:
    print(f"  {addr}: {sig} | calls={calls}")
