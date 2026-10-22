"""Find trivial leaf functions whose decrypted Ghidra bytes prove their body.

The project is read only. No C++ is generated from a decompiler guess alone.
"""
from __future__ import annotations

import argparse
import csv
import json
import re
import struct
from pathlib import Path

import pyghidra
from jpype import JArray, JByte
import yaml


def classify(data: bytes, decompiled: str):
    body = re.sub(r"\s+", " ", decompiled).strip()
    if data[0] == 0xC3 and re.search(r"\{ return; \}", body):
        return "empty", 0, 1
    if data[:3] == b"\xC2\x00\x00" and re.search(r"\{ return; \}", body):
        return "empty", 0, 3
    if data[0] == 0xB8 and data[5] == 0xC3:
        value = struct.unpack_from("<I", data, 1)[0]
        if f"return {value};" in body or f"return 0x{value:x};" in body:
            return "constant32", value, 6
    if data[:3] in (b"\x31\xc0\xc3", b"\x33\xc0\xc3") and "return 0;" in body:
        return "constant32", 0, 3
    if data[0] == 0xB0 and data[2] == 0xC3:
        value = data[1]
        if f"return {value};" in body or f"return 0x{value:x};" in body:
            return "constant8", value, 3
    if data[:5] == b"\x8b\x44\x24\x04\xc3" and "return param_1;" in body:
        return "identity32", 0, 5
    if data[0] == 0xA1 and data[5] == 0xC3:
        address = struct.unpack_from("<I", data, 1)[0]
        if re.search(rf"return _?DAT_{address:08x};", body, re.IGNORECASE):
            return "load32", address, 6
    if data[:3] == b"\x0f\xb6\x05" and data[7] == 0xC3:
        address = struct.unpack_from("<I", data, 3)[0]
        if re.search(rf"return _?DAT_{address:08x};", body, re.IGNORECASE):
            return "load8", address, 8
    if data[:2] == b"\x8a\x05" and data[6] == 0xC3:
        address = struct.unpack_from("<I", data, 2)[0]
        if re.search(rf"return _?DAT_{address:08x};", body, re.IGNORECASE):
            return "load8", address, 7
    if data[:2] == b"\xc7\x05" and data[10] == 0xC3:
        address, value = struct.unpack_from("<II", data, 2)
        if value == 0 and re.search(rf"_?DAT_{address:08x} = 0;", body, re.IGNORECASE):
            return "store_zero32", address, 11
    if data[:2] == b"\xc6\x05" and data[7] == 0xC3:
        address = struct.unpack_from("<I", data, 2)[0]
        if data[6] == 0 and re.search(rf"_?DAT_{address:08x} = 0;", body, re.IGNORECASE):
            return "store_zero8", address, 8
    if data[:3] == b"\x8b\xc1\xc3" and "return param_1;" in body:
        return "identity_ecx", 0, 3
    if data[:7] == b"\x8b\x44\x24\x04\x8b\x00\xc3" and "return *param_1;" in body:
        return "deref32", 0, 7
    if data[:2] == b"\x8b\x41" and data[3] == 0xC3:
        offset = data[2]
        if re.search(rf"return \*\(undefined4 \*\)\(param_1 \+ (?:{offset}|0x{offset:x})\);", body):
            return "member_load32", offset, 4
    if data[:2] == b"\x8a\x41" and data[3] == 0xC3:
        offset = data[2]
        if re.search(rf"return \*\(undefined1 \*\)\(param_1 \+ (?:{offset}|0x{offset:x})\);", body):
            return "member_load8", offset, 4
    if data[:2] == b"\xc7\x41" and data[7] in (0xC3, 0xC2):
        offset = data[2]
        value = struct.unpack_from("<I", data, 3)[0]
        if re.search(rf"\*\(undefined4 \*\)\(param_1 \+ (?:{offset}|0x{offset:x})\) = (?:{value}|0x{value:x});", body):
            length = 8 if data[7] == 0xC3 else 10 if data[8:10] == b"\x04\x00" else 0
            if length:
                return "member_store32", (offset << 32) | value, length
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--config", type=Path, default=Path("ghidra-bridge.yaml"))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    config = yaml.safe_load(args.config.read_text(encoding="utf-8"))["ghidra"]
    tracked = {
        row["address"] for row in csv.DictReader(
            Path("architecture/address_map.csv").open(encoding="utf-8", newline="")
        ) if row["integration"] == "integrated"
    }
    candidates = []
    for path in Path(".ghidra-exports").glob("*.json"):
        if path.stem in tracked:
            continue
        record = json.loads(path.read_text(encoding="utf-8"))
        decompiled = record.get("decompiled", "")
        if (not decompiled or len(decompiled) > 500 or record.get("is_thunk")
                or record.get("callees") or record.get("parameter_count", 0) > 1):
            continue
        candidates.append((path.stem, record))

    pyghidra.start(install_dir=Path(config["install_dir"]))
    project = pyghidra.open_project(Path(config["project_dir"]), config["project_name"])
    program, consumer = pyghidra.consume_program(project, "/" + config["program_name"])
    try:
        address_space = program.getAddressFactory().getDefaultAddressSpace()
        functions = program.getFunctionManager()
        found = []
        for address, record in candidates:
            java_address = address_space.getAddress(int(address, 16))
            function = functions.getFunctionAt(java_address)
            if function is None:
                continue
            raw = JArray(JByte)(16)
            if program.getMemory().getBytes(java_address, raw) != 16:
                continue
            data = bytes(value & 0xff for value in raw)
            result = classify(data, record["decompiled"])
            if result is None:
                continue
            kind, value, length = result
            if function.getBody().getNumAddresses() != length:
                continue
            found.append({
                "address": address,
                "kind": kind,
                "value": value,
                "bytes": data[:length].hex(),
                "signature": record.get("signature", ""),
                "name": record.get("name", ""),
            })
        found.sort(key=lambda item: item["address"])
        report = {
            "program": str(program.getName()),
            "executable_sha256": str(program.getExecutableSHA256()),
            "functions": found,
        }
        if args.output:
            args.output.write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
        print("program", report["program"])
        print("executable_sha256", report["executable_sha256"])
        print("verified_small_leafs", len(found))
        for item in found:
            print(item["address"], item["kind"], item["value"], item["bytes"], item["signature"])
    finally:
        program.release(consumer)
        project.close()


if __name__ == "__main__":
    main()
