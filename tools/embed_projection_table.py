"""Generate the C++ initializer from the extracted 256-byte Ghidra table."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
source = ROOT / "architecture" / "projection_format_table.bin"
target = ROOT / "src" / "projection" / "FormatTable.inc"
data = source.read_bytes()
if len(data) != 256:
    raise ValueError(f"Expected 256 bytes from 0x0071b150, found {len(data)}")
lines = [
    "// Ghidra project hp6.exe, memory 0x0071b150, 256 bytes.",
    "// Generated from architecture/projection_format_table.bin.",
]
lines.extend(
    "    " + ", ".join(f"0x{value:02x}" for value in data[i:i + 16]) + ","
    for i in range(0, len(data), 16)
)
target.write_text("\n".join(lines) + "\n", encoding="utf-8")
