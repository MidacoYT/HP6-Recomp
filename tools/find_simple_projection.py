import json
from pathlib import Path

state = json.loads(Path("reports/pipeline/state.json").read_text(encoding="utf-8"))
integrated = set(a.lower() for a, e in state.get("functions", {}).items() if e.get("integration") == "integrated")

# Zone projection proche de 0x006d6000-0x006d7000
lo, hi = 0x006d6000, 0x006d7000

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
        asm_len = len(d.get("assembly", []))
        # Fonctions feuilles simples (pas d'appels, petit assembleur)
        if len(calls) == 0 and asm_len < 60:
            candidates.append((addr_str, sig, asm_len))

candidates.sort(key=lambda x: x[2])  # par taille asm croissante
print(f"{len(candidates)} fonctions feuilles simples dans 0x006d6000-0x006d7000")
for addr, sig, asm_len in candidates[:20]:
    print(f"  {addr}: {sig} | asm={asm_len}")
