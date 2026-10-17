import json
from pathlib import Path

candidates = ["00401000", "00401270", "004013a0", "00401da0", "00403190"]

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
