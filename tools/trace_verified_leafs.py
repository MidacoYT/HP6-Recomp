"""Trace verified HP6 leafs to direct callers and PE data pointer references."""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

from audit_native_globals import read_pe, bytes_at
from disasm_original import UNPROTECTED_SHA256


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--source", type=Path, default=Path("src/runtime/VerifiedBinaryLeafs.cpp"))
    parser.add_argument("--output", type=Path)
    parser.add_argument("--details", action="store_true")
    args = parser.parse_args()
    image, base, _, sections = read_pe(args.binary)
    if hashlib.sha256(image).hexdigest() != UNPROTECTED_SHA256:
        raise ValueError("the supplied image is not the verified unprotected HP6 binary")
    addresses = sorted(set(re.findall(r"// 0x([0-9a-f]{8}):", args.source.read_text()))
                       | {"006d8f90", "006c1d20", "006be290"})  # Named in platform/audio.
    result = []
    for address_hex in addresses:
        address = int(address_hex, 16)
        export = json.loads((Path(".ghidra-exports") / f"{address_hex}.json").read_text())
        referrers = sorted({(caller["addr"], caller["ref_type"])
                           for caller in export["callers"]})
        pointers = []
        for name, _, rva, raw_size, raw_offset in sections:
            if name not in (".rdata", ".data"):
                continue
            section = image[raw_offset:raw_offset + raw_size]
            target = struct.pack("<I", address)
            start = 0
            while (offset := section.find(target, start)) >= 0:
                start = offset + 1
                if offset % 4:
                    continue
                pointer_address = base + rva + offset
                neighbors = []
                for delta in (-8, -4, 4, 8):
                    _, raw = bytes_at(image, base, sections, pointer_address + delta, 4)
                    neighbors.append(int.from_bytes(raw, "little") if raw else None)
                pointers.append({"address": f"{pointer_address:08x}", "section": name,
                                 "neighbors": [f"{value:08x}" if value is not None else None
                                               for value in neighbors]})
        result.append({"address": address_hex,
                       "referrers": [{"address": addr, "type": kind}
                                     for addr, kind in referrers],
                       "pointers": pointers})
    if args.output:
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    for item in result:
        print(item["address"], len(item["referrers"]), "Ghidra referrers,",
              len(item["pointers"]), "data pointers")
        if args.details:
            for referrer in item["referrers"][:8]:
                caller = referrer["address"]
                path = Path(".ghidra-exports") / f"{caller}.json"
                if not path.exists():
                    continue
                record = json.loads(path.read_text())
                snippets = [line.strip() for line in record["decompiled"].splitlines()
                            if f"FUN_{item['address']}" in line]
                print("  ", caller, referrer["type"], record["signature"],
                      *snippets[:3], sep=" | ")


if __name__ == "__main__":
    main()
