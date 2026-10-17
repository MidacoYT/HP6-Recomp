import json
from pathlib import Path

candidates = [
    "006d793d", "006d7980", "006d79fa",
    "006d7d9c", "006d7dea",
    "006d6ab6", "006d6d39",
    "006d81d8", "006d827e",
]

for addr in candidates:
    p = Path(f".ghidra-exports/{addr}.json")
    if not p.exists():
        print(f"{addr}: PAS D'EXPORT")
        continue
    d = json.loads(p.read_text(encoding="utf-8"))
    sig = d.get("signature", "?")
    calls = d.get("calls", [])
    asm_len = len(d.get("assembly", ""))
    print(f"{addr}: {sig} | calls={calls} | asm_len={asm_len}")
