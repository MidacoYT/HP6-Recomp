import json
from pathlib import Path

deferred = ["005d16f0", "005e8b50", "0068c0a0"]

for addr in deferred:
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
    print(f"asm_len={len(asm)}")
    for line in asm[:20]:
        print(f"  {line}")
    if len(asm) > 20:
        print(f"  ... ({len(asm)-20} more lines)")
