"""Disassemble a small range from either verified HP6 PC executable image."""

import argparse
from pathlib import Path

import capstone

from audit_native_globals import ORIGINAL_SHA256, bytes_at, read_pe
import hashlib

UNPROTECTED_SHA256 = "41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10"


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=Path)
    parser.add_argument("address", type=lambda value: int(value, 0))
    parser.add_argument("--size", type=lambda value: int(value, 0), default=0x80)
    args = parser.parse_args()
    image, base, _, sections = read_pe(args.binary)
    digest = hashlib.sha256(image).hexdigest()
    if digest not in (ORIGINAL_SHA256, UNPROTECTED_SHA256):
        raise ValueError("binary hash differs from both verified HP6 images")
    section, data = bytes_at(image, base, sections, args.address, args.size)
    if data is None:
        raise ValueError("address outside mapped PE sections")
    print(f"{args.address:#010x} {section} SHA-256 {digest}")
    decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)
    for instruction in decoder.disasm(data, args.address):
        print(f"{instruction.address:08x}  {instruction.bytes.hex():<20} "
              f"{instruction.mnemonic:<8} {instruction.op_str}")


if __name__ == "__main__":
    main()
