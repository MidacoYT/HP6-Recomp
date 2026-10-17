from pathlib import Path

exports = sorted(p.stem for p in Path(".ghidra-exports").glob("*.json"))
print(f"{len(exports)} exports disponibles")
print(" ".join(exports))
