"""Inspect unresolved native table words in the two verified HP6 PEs."""

import hashlib
import struct
from pathlib import Path

from audit_native_globals import ORIGINAL_SHA256, bytes_at, read_pe
from disasm_original import UNPROTECTED_SHA256


TABLES = {
    "service_holder_vtables_0083e55c": (0x0083E55C, 9),
    "base_vtable_008201f8": (0x008201F8, 4),
    "command_pointers_00876ab0": (0x00876AB0, 93),
    "screen_pointers_00870948": (0x00870948, 26),
}


def words(path: Path, expected_hash: str, address: int, count: int):
    image, base, _, sections = read_pe(path)
    digest = hashlib.sha256(image).hexdigest()
    if digest != expected_hash:
        raise ValueError(f"unexpected SHA-256 for {path}: {digest}")
    section, data = bytes_at(image, base, sections, address, 4 * count)
    if data is None:
        raise ValueError(f"{address:#x} outside PE sections")
    return struct.unpack(f"<{count}I", data)


def main():
    original = Path(r"D:\HP\hp6.exe")
    unprotected = Path(r"D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe")
    for name, (address, count) in TABLES.items():
        a = words(original, ORIGINAL_SHA256, address, count)
        b = words(unprotected, UNPROTECTED_SHA256, address, count)
        print(f"{name}: {count} words, same={a == b}")
        for index, (x, y) in enumerate(zip(a, b)):
            if name.endswith("pointers_00876ab0") or name.endswith("pointers_00870948"):
                if x != y:
                    print(f"  {index:2d}: original={x:08x}, unprotected={y:08x}")
            else:
                print(f"  {index:2d}: {x:08x}" + (f" / {y:08x}" if x != y else ""))

    print("screen object vtables:")
    import csv
    with Path("architecture/screen_task_table.csv").open(newline="", encoding="utf-8") as handle:
        for entry in csv.DictReader(handle):
            address = int(entry["vtable_address"], 16)
            a = words(original, ORIGINAL_SHA256, address, 4)
            b = words(unprotected, UNPROTECTED_SHA256, address, 4)
            print(f"  {entry['screen']}: {', '.join(f'{x:08x}' for x in a)} same={a == b}")


if __name__ == "__main__":
    main()
