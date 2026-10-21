"""Disassemble a small byte range from the verified original HP6 PE image."""

import argparse
from pathlib import Path

import capstone

from audit_native_globals import ORIGINAL_SHA256, bytes_at, read_pe
import hashlib


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=Path)
    parser.add_argument("address", type=lambda value: int(value, 0))
    parser.add_argument("--size", type=lambda value: int(value, 0), default=0x80)
    args = parser.parse_args()
    image, base, _, sections = read_pe(args.binary)
    if hashlib.sha256(image).hexdigest() != ORIGINAL_SHA256:
        raise ValueError("binary hash differs from the verified original")
    section, data = bytes_at(image, base, sections, args.address, args.size)
    if data is None:
        raise ValueError("address outside mapped PE sections")
    print(f"{args.address:#010x} {section}")
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for instruction in decoder.disasm(data, args.address):
        print(f"{instruction.address:08x}  {instruction.bytes.hex():<20} "
              f"{instruction.mnemonic:<8} {instruction.op_str}")


if __name__ == "__main__":
    main()
