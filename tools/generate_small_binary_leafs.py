"""Generate exact small leaf reconstructions from a verified HP6 PC binary.

The input report is produced by find_small_binary_leafs.py from the Ghidra
project for the unprotected executable. Every instruction sequence is checked
again against the matching on-disk executable before C++ is emitted.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
from pathlib import Path

from audit_native_globals import bytes_at, read_pe
from disasm_original import UNPROTECTED_SHA256

# Debug-build leafs with prologues or two-field arithmetic. These exact byte
# sequences were read from the same unprotected executable and checked
# against their Ghidra decompilations. The first seven receive ECX as input.
EXTRA_LEAFS = (
    ("006bdfa0", "member_load32", 0x04, "558bec51c745fccccccccc894dfc8b45fc8b40048be55dc3"),
    ("006bde50", "member_load32", 0x64, "558bec51c745fccccccccc894dfc8b45fc8b40648be55dc3"),
    ("006bdf50", "member_load32", 0x14, "558bec51c745fccccccccc894dfc8b45fc8b40148be55dc3"),
    ("006bdfc0", "member_load32", 0x24, "558bec51c745fccccccccc894dfc8b45fc8b40248be55dc3"),
    ("006be290", "member_load32", 0x10, "558bec51c745fccccccccc894dfc8b45fc8b40108be55dc3"),
    ("006c1d00", "member_load32", 0x58, "558bec51c745fccccccccc894dfc8b45fc8b40588be55dc3"),
    ("006c1d20", "member_load32", 0x14, "558bec51c745fccccccccc894dfc8b45fc8b40148be55dc3"),
    ("006c9e80", "stack_load32", 0x244, "558bec8b45088b80440200005dc3"),
    ("006cec20", "stack_load8", 0x1ac, "558bec8b45088a80ac0100005dc3"),
    ("006c9e90", "stack_sum32", 0x220, "558bec8b4d088b81440200000381200200005dc3"),
    ("006c9eb0", "stack_sum32", 0x224, "558bec8b4d088b81440200000381240200005dc3"),
    ("006b477f", "clear2", 0, "8bc183200083600400c3"),
    ("006e712e", "clear3", 0, "8b44240433c98908894804894808c3"),
)

# These labels describe observed uses, not recovered original C++ symbol names.
ORIGIN_CONTEXT = {
    "00401220": "MSVC array-constructor callback; returns the ECX object pointer",
    "00405a30": "MSVC array-destructor callback and other empty virtual methods",
    "00407b30": "virtual method returning 1; used by finite-state registration",
    "004d59c0": "virtual method returning 2; used by finite-state registration",
    "004eef20": "virtual method returning 3; used by finite-state registration",
    "00622a70": "virtual method returning 4; used by finite-state registration",
    "0050c0c0": "virtual hook reached from list/task removal at 005d2c10",
    "006b477f": "two-word initializer in command handling and MemCard construction",
    "006bde50": "PATH audio virtual table; reads object field +0x64",
    "006bdf50": "PATH audio virtual table; reads object field +0x14",
    "006bdfa0": "PATH audio virtual table; reads object field +0x04",
    "006bdfc0": "PATH audio virtual table; reads object field +0x24",
    "006be290": "PATH audio virtual table; reads object field +0x10",
    "006c1d00": "PATH audio virtual table; reads object field +0x58",
    "006c1d20": "PATH audio virtual table; reads object field +0x14",
    "006cec20": "byte getter paired with setter 006cec00 at field +0x1ac",
    "006de5de": "zero result queried by the input polling functions",
    "006e208d": "empty callback registered and called during audio update",
    "006e6016": "zero-valued helper used by audio buffer offset calculations",
    "006e712e": "clears two three-word records during audio buffer setup",
}


def decompilation_matches(kind: str, value: int, decompiled: str) -> bool:
    body = re.sub(r"\s+", " ", decompiled).strip()
    offset = rf"(?:{value}|0x{value:x})"
    if kind in ("member_load32", "stack_load32"):
        return bool(re.search(rf"return \*\(undefined4 \*\)\(param_1 \+ {offset}\);", body))
    if kind == "stack_load8":
        return bool(re.search(rf"return \*\(undefined1 \*\)\(param_1 \+ {offset}\);", body))
    if kind == "stack_sum32":
        return bool(re.search(
            rf"return \*\(int \*\)\(param_1 \+ 0x244\) \+ \*\(int \*\)\(param_1 \+ {offset}\);", body
        ))
    if kind in ("clear2", "clear3"):
        expected = "*param_1 = 0; param_1[1] = 0; "
        if kind == "clear3":
            expected += "param_1[2] = 0; "
        return expected + "return;" in body
    return False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("report", type=Path)
    parser.add_argument("header", type=Path)
    parser.add_argument("source", type=Path)
    args = parser.parse_args()

    image, base, _, sections = read_pe(args.binary)
    digest = hashlib.sha256(image).hexdigest()
    report = json.loads(args.report.read_text(encoding="utf-8"))
    if digest != UNPROTECTED_SHA256 or report["executable_sha256"] != digest:
        raise ValueError("report does not match the verified unprotected HP6 executable")

    entries = []
    for item in report["functions"]:
        if item["address"] in ("006d8f90",):
            continue  # named lock-depth accessor now lives in platform/SyncAccessors.cpp
        if item["kind"] == "member_store32" and item["bytes"].endswith("c20400"):
            continue  # hidden stack argument: ABI still under review
        if item["kind"] == "store_zero32":
            continue  # storage type and aliasing still need individual audit
        if item["kind"] == "load32" and item["value"] != 0x00998580:
            continue  # only this global already has a shared, typed definition
        address = int(item["address"], 16)
        code = bytes.fromhex(item["bytes"])
        section, actual = bytes_at(image, base, sections, address, len(code))
        if section != ".text" or actual != code:
            raise ValueError(f"binary bytes changed at {address:08x}")
        entries.append(item)
    for address_hex, kind, value, expected_hex in EXTRA_LEAFS:
        address = int(address_hex, 16)
        expected = bytes.fromhex(expected_hex)
        section, actual = bytes_at(image, base, sections, address, len(expected))
        export = json.loads((Path(".ghidra-exports") / f"{address_hex}.json").read_text(encoding="utf-8"))
        if (section != ".text" or actual != expected or export.get("callees") or
                export.get("is_thunk") or not decompilation_matches(kind, value, export["decompiled"])):
            raise ValueError(f"extra leaf is not verified at {address_hex}")
        if address_hex in ("006c1d20", "006be290"):
            continue  # named PATH volume/dry-level accessors live in audio/PathVolume.cpp
        entries.append({"address": address_hex, "kind": kind, "value": value,
                        "bytes": expected_hex})
    entries.sort(key=lambda item: item["address"])

    header = [
        "// Generated by tools/generate_small_binary_leafs.py from HP6 PC code.",
        f"// Unprotected executable SHA-256: {digest}",
        "#pragma once",
        "#include <cstdint>",
        "#if defined(_MSC_VER) && defined(_M_IX86)",
        "#define HP6_LEAF_FASTCALL __fastcall",
        "#define HP6_LEAF_CDECL __cdecl",
        "#else",
        "#define HP6_LEAF_FASTCALL",
        "#define HP6_LEAF_CDECL",
        "#endif",
        "namespace hp6::binary_leafs {",
    ]
    source = [
        "// Generated by tools/generate_small_binary_leafs.py from HP6 PC code.",
        f"// Unprotected executable SHA-256: {digest}",
        '#include "runtime/VerifiedBinaryLeafs.h"',
        "",
    ]
    globals_used = {}
    definitions = []
    for item in entries:
        kind = item["kind"]
        address = item["address"]
        name = "fn_" + address
        value = item["value"]
        if kind in ("load32", "store_zero32"):
            global_name = f"DAT_{value:08x}"
            globals_used[global_name] = "std::int32_t" if value == 0x00998580 else "std::uint32_t"
        if kind in ("empty", "store_zero32"):
            signature = f"void {name}()"
            body = "return;" if kind == "empty" else f"{global_name} = 0;"
        elif kind == "constant32":
            signature = f"std::uint32_t {name}()"
            body = f"return 0x{value:08x}u;"
        elif kind == "load32":
            signature = f"std::uint32_t {name}()"
            body = f"return static_cast<std::uint32_t>({global_name});"
        elif kind in ("identity_ecx", "identity32"):
            convention = "HP6_LEAF_FASTCALL " if kind == "identity_ecx" else "HP6_LEAF_CDECL "
            signature = f"std::uint32_t {convention}{name}(std::uint32_t value)"
            body = "return value;"
        elif kind == "deref32":
            signature = f"std::uint32_t HP6_LEAF_CDECL {name}(const std::uint32_t* value)"
            body = "return *value;"
        elif kind == "member_load32":
            signature = f"std::uint32_t HP6_LEAF_FASTCALL {name}(const std::uint8_t* object)"
            body = f"return *reinterpret_cast<const std::uint32_t*>(object + 0x{value:x});"
        elif kind == "member_load8":
            signature = f"std::uint8_t HP6_LEAF_FASTCALL {name}(const std::uint8_t* object)"
            body = f"return object[0x{value:x}];"
        elif kind == "member_store32":
            offset, stored = value >> 32, value & 0xffffffff
            signature = f"void HP6_LEAF_FASTCALL {name}(std::uint8_t* object)"
            body = f"*reinterpret_cast<std::uint32_t*>(object + 0x{offset:x}) = 0x{stored:08x}u;"
        elif kind == "stack_load32":
            signature = f"std::uint32_t HP6_LEAF_CDECL {name}(const std::uint8_t* object)"
            body = f"return *reinterpret_cast<const std::uint32_t*>(object + 0x{value:x});"
        elif kind == "stack_load8":
            signature = f"std::uint8_t HP6_LEAF_CDECL {name}(const std::uint8_t* object)"
            body = f"return object[0x{value:x}];"
        elif kind == "stack_sum32":
            signature = f"std::uint32_t HP6_LEAF_CDECL {name}(const std::uint8_t* object)"
            body = ("return *reinterpret_cast<const std::uint32_t*>(object + 0x244) + "
                    f"*reinterpret_cast<const std::uint32_t*>(object + 0x{value:x});")
        elif kind in ("clear2", "clear3"):
            convention = "HP6_LEAF_FASTCALL" if kind == "clear2" else "HP6_LEAF_CDECL"
            signature = f"void {convention} {name}(std::uint32_t* object)"
            count = 2 if kind == "clear2" else 3
            operation = "&= 0" if kind == "clear2" else "= 0"
            body = " ".join(f"object[{index}] {operation};" for index in range(count))
        else:
            raise ValueError(f"unhandled classified opcode at {address}: {kind}")
        header.append(signature + ";")
        definitions.extend([
            *([f"// Observed use: {ORIGIN_CONTEXT[address]}"] if address in ORIGIN_CONTEXT else []),
            f"// 0x{address}: {item['bytes']} ({kind})",
            signature + " { " + body + " }",
        ])

    header.extend(["} // namespace hp6::binary_leafs", ""])
    for name, c_type in sorted(globals_used.items()):
        source.append(f"extern {c_type} {name};")
    source.extend(["", "namespace hp6::binary_leafs {", *definitions,
                   "} // namespace hp6::binary_leafs", ""])
    args.header.write_text("\n".join(header), encoding="utf-8")
    args.source.write_text("\n".join(source), encoding="utf-8")
    print(f"generated {len(entries)} verified leaf functions")


if __name__ == "__main__":
    main()
