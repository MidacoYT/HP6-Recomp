"""Verify the native command and screen-task records in both known HP6 PEs.

The CSV files are inventories of binary pointers, not callable runtime tables.
"""

import argparse
import csv
import hashlib
import struct
from pathlib import Path

from audit_native_globals import ORIGINAL_SHA256, bytes_at, read_pe
from disasm_original import UNPROTECTED_SHA256


def image_at(path: Path, expected_hash: str):
    image, base, _, sections = read_pe(path)
    actual = hashlib.sha256(image).hexdigest()
    if actual != expected_hash:
        raise ValueError(f"unexpected SHA-256 for {path}: {actual}")

    def data(address: int, size: int) -> bytes:
        section, value = bytes_at(image, base, sections, address, size)
        if value is None:
            raise ValueError(f"{address:#x} is outside the PE sections")
        return value

    def word(address: int) -> int:
        return struct.unpack("<I", data(address, 4))[0]

    def c_string(address: int) -> str:
        return data(address, 256).split(b"\0", 1)[0].decode("cp1252")

    commands = []
    for index in range(93):
        record = word(0x00876AB0 + 4 * index)
        name, handler = struct.unpack("<II", data(record, 8))
        commands.append((index, c_string(name), f"{handler:08x}"))

    screens = []
    for index in range(26):
        record = word(0x00870948 + 4 * index)
        vtable, name, state = struct.unpack("<III", data(record, 12))
        screens.append((index, c_string(name), f"{record:08x}",
                        f"{vtable:08x}", f"{word(vtable):08x}", state))
    return commands, screens


def read_csv(path: Path):
    with path.open(newline="", encoding="utf-8") as handle:
        return list(csv.reader(handle))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("original", type=Path)
    parser.add_argument("unprotected", type=Path)
    parser.add_argument("--write-screens", type=Path)
    args = parser.parse_args()
    original = image_at(args.original, ORIGINAL_SHA256)
    unprotected = image_at(args.unprotected, UNPROTECTED_SHA256)
    if original != unprotected:
        raise ValueError("dispatch table records differ between the verified PEs")

    commands, screens = original
    expected = [["index", "command", "handler_address"]] + [
        [str(index), name, handler] for index, name, handler in commands]
    existing = read_csv(Path(__file__).resolve().parent.parent /
                        "architecture/presentation_commands.csv")
    if existing != expected:
        raise ValueError("presentation_commands.csv differs from both PEs")

    expected_screens = [["index", "screen", "record_address", "vtable_address",
                         "activate_address", "initial_state"]] + [
        [str(index), name, record, vtable, activate, str(state)]
        for index, name, record, vtable, activate, state in screens]
    screen_csv = Path(__file__).resolve().parent.parent / "architecture/screen_task_table.csv"
    if screen_csv.exists() and read_csv(screen_csv) != expected_screens:
        raise ValueError("screen_task_table.csv differs from both PEs")

    if args.write_screens:
        with args.write_screens.open("w", newline="", encoding="utf-8") as handle:
            writer = csv.writer(handle, lineterminator="\n")
            writer.writerow(["index", "screen", "record_address", "vtable_address",
                             "activate_address", "initial_state"])
            writer.writerows(screens)
    print(f"verified {len(commands)} commands and {len(screens)} screen tasks in both PEs")


if __name__ == "__main__":
    main()
