import json
from pathlib import Path

candidates = ["006d6ab6", "006d6d39"]

for addr in candidates:
    p = Path(f".ghidra-exports/{addr}.json")
    if not p.exists():
        print(f"{addr}: PAS D'EXPORT")
        continue
    d = json.loads(p.read_text(encoding="utf-8"))
    sig = d.get("signature", "?")
    calls = d.get("calls", [])
    asm = d.get("assembly", "")
    print(f"\n===== {addr}: {sig} =====")
    print(f"calls={calls}")
    print(asm[:800])

# Check 0x006b938c
p = Path(".ghidra-exports/006b938c.json")
if p.exists():
    d = json.loads(p.read_text(encoding="utf-8"))
    print(f"\n===== 006b938c: {d.get('signature', '?')} =====")
    print(f"calls={d.get('calls', [])}")
else:
    print("\n006b938c: PAS D'EXPORT")