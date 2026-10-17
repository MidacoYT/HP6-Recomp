import json
from pathlib import Path

candidates = ["006d6ab0", "006d6aa3", "006d6b3f", "006d6b54", "006d6040"]

for addr in candidates:
    p = Path(f".ghidra-exports/{addr}.json")
    if not p.exists():
        print(f"{addr}: PAS D'EXPORT")
        continue
    d = json.loads(p.read_text(encoding="utf-8"))
    sig = d.get("signature", "?")
    calls = d.get("calls", [])
    asm = d.get("assembly", [])
    print(f"\n===== {addr}: {sig} =====")
    print(f"calls={calls}")
    for line in asm:
        print(f"  {line}")
