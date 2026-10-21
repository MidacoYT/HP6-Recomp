"""Audit unresolved native globals against the exact original HP6 PE image.

This is an analysis tool, not a game runtime implementation. It never writes
symbols or substitutes values from smoke tests into the reconstructed game.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import json
import re
import struct
from pathlib import Path

ORIGINAL_SHA256 = "4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954"

# Extents observed in the reconstructed callers' decompiled zero-fill,
# indexed loops, or fixed data structures. The entire PE range is checked
# before any declaration is emitted. Overlapping inner Ghidra labels are
# deliberately left unresolved until they share storage with their base.
IMAGE_ARRAYS = (
    ("std::int32_t", "DAT_00851380", 20, 4),
    ("std::uint32_t", "DAT_00881370", 19, 4),
    ("std::uint32_t", "DAT_00892b80", 26, 4),
    ("std::int32_t", "DAT_008928c0", 3, 4),
    ("std::int32_t", "DAT_0096b468", 6, 4),
    ("std::int32_t", "DAT_009968d8", 257, 4),
    ("std::uint8_t", "DAT_0098c6a8", 64 * 32 * 0x14, 1),
    ("std::uint8_t", "DAT_009985a0", 32 * 0x14, 1),
    ("std::int32_t", "DAT_00a70860", 6, 4),
    ("std::uint8_t", "DAT_00a70890", 64 * 512 * 0x14, 1),
    ("ShaderSlot", "DAT_00a68820", 4, 8),
    ("std::int32_t", "DAT_00b108a0", 257, 4),
    ("std::uint8_t", "DAT_00baa170", 16 * 32 * 0x14, 1),
    ("std::uint32_t", "DAT_00bb09b4", 16, 4),
    ("std::int32_t", "DAT_00c75188", 236, 4),
    ("std::uint32_t", "DAT_00c75538", 14, 4),
    ("char", "DAT_00d63f68", 0x200, 1),
    ("char", "DAT_00d64168", 0x200, 1),
    ("char*", "DAT_00d64368", 0x20, 4),
    ("char*", "DAT_00d643e8", 0x20, 4),
    ("char*", "DAT_00d64468", 0x20, 4),
    ("std::uint8_t", "DAT_00d71250", 0x268, 1),
    ("std::uint8_t", "DAT_00f38b10", 0x600 * 0xC4, 1),
)


def read_pe(path: Path):
    image = path.read_bytes()
    pe = struct.unpack_from("<I", image, 0x3C)[0]
    if image[pe : pe + 4] != b"PE\0\0":
        raise ValueError("not a PE image")
    count = struct.unpack_from("<H", image, pe + 6)[0]
    optional_size = struct.unpack_from("<H", image, pe + 20)[0]
    optional = pe + 24
    if struct.unpack_from("<H", image, optional)[0] != 0x10B:
        raise ValueError("expected the original 32-bit PE image")
    image_base = struct.unpack_from("<I", image, optional + 28)[0]
    entry_rva = struct.unpack_from("<I", image, optional + 16)[0]
    sections = []
    first_section = optional + optional_size
    for index in range(count):
        offset = first_section + index * 40
        name = image[offset : offset + 8].split(b"\0", 1)[0].decode("ascii")
        virtual_size, rva, raw_size, raw_offset = struct.unpack_from(
            "<4I", image, offset + 8
        )
        sections.append((name, virtual_size, rva, raw_size, raw_offset))
    return image, image_base, entry_rva, sections


def bytes_at(image: bytes, base: int, sections: list, address: int, size: int):
    rva = address - base
    for name, virtual_size, start_rva, raw_size, raw_offset in sections:
        if start_rva <= rva and rva + size <= start_rva + max(virtual_size, raw_size):
            offset = rva - start_rva
            if offset + size <= raw_size:
                return name, image[raw_offset + offset : raw_offset + offset + size]
            if offset >= raw_size:
                return name, bytes(size)
            present = image[raw_offset + offset : raw_offset + raw_size]
            return name, present + bytes(size - len(present))
    return None, None


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("binary", type=Path)
    parser.add_argument("link_log", type=Path)
    parser.add_argument("--json", action="store_true")
    parser.add_argument("--cpp-output", type=Path)
    parser.add_argument("--cpp-arrays-output", type=Path)
    parser.add_argument("--cpp-chapters-output", type=Path)
    parser.add_argument("--cpp-drive-table-output", type=Path)
    parser.add_argument("--cpp-heap-overrides-output", type=Path)
    args = parser.parse_args()
    image, base, entry_rva, sections = read_pe(args.binary)
    digest = hashlib.sha256(image).hexdigest()
    if digest != ORIGINAL_SHA256:
        raise ValueError(f"different hp6.exe image: {digest}")
    log_bytes = args.link_log.read_bytes()
    log = log_bytes.decode("utf-16" if log_bytes.startswith((b"\xff\xfe", b"\xfe\xff"))
                           else "utf-8", errors="replace")
    symbols = sorted(
        set(re.findall(r'error LNK20(?:01|19):[^\n]*?"([^"]+)"', log))
    )
    result = []
    for symbol in symbols:
        match = re.match(r"(.+?)\s+((?:_?DAT_|PTR_)[A-Za-z0-9_]+)$", symbol)
        if not match:
            result.append({"symbol": symbol, "kind": "function"})
            continue
        c_type, name = match.groups()
        address_match = re.search(r"([0-9a-fA-F]{8})$", name)
        address = int(address_match.group(1), 16) if address_match else None
        size = 1 if c_type == "unsigned char" else 4
        section, raw = bytes_at(image, base, sections, address, size)
        item = {
            "symbol": name,
            "type": c_type,
            "address": f"0x{address:08X}",
            "section": section,
            "raw_hex": raw.hex() if raw is not None else None,
            "kind": "pointer_or_array" if "*" in c_type else "scalar",
        }
        if raw is not None and item["kind"] == "scalar":
            if c_type == "float":
                item["value"] = struct.unpack("<f", raw)[0]
            else:
                item["value"] = int.from_bytes(raw, "little", signed=c_type == "int")
        result.append(item)
    summary = dict(collections.Counter(item["kind"] for item in result))
    report = {
        "binary_sha256": digest,
        "pe_entry": f"0x{base + entry_rva:08X}",
        "symbol_counts": summary,
        "symbols": result,
    }
    if args.cpp_output:
        lines = [
            "// Generated by tools/audit_native_globals.py from the original PE.",
            f"// SHA-256: {report['binary_sha256']}",
            "// Initial scalars and zero-valued void-pointer slots only.",
            "// Arrays, nonzero pointers and services require separate reconstruction",
            "// from their decompiled users before the game can start.",
            "#include <bit>",
            "#include <cstdint>",
            "",
        ]
        for item in sorted(result, key=lambda entry: entry.get("address", "")):
            zero_pointer = (
                item["kind"] == "pointer_or_array"
                and (item["type"] in ("void *", "void * *")
                     or item["symbol"] == "DAT_00fcd534")
                and item["raw_hex"] == "00000000"
            )
            if item["kind"] != "scalar" and not zero_pointer:
                continue
            kind = item["type"]
            raw = item["raw_hex"]
            if raw is None:
                raise ValueError(f"no original bytes for {item['symbol']}")
            word = int.from_bytes(bytes.fromhex(raw), "little")
            if zero_pointer:
                cpp_type = ("void**" if kind == "void * *" else
                            "std::uint8_t*" if item["symbol"] == "DAT_00fcd534" else
                            "void*")
                declaration = f"{cpp_type} {item['symbol']} = nullptr;"
            elif kind == "float":
                declaration = f"float {item['symbol']} = std::bit_cast<float>(0x{word:08X}u);"
            elif kind == "int":
                declaration = (
                    f"std::int32_t {item['symbol']} = "
                    f"static_cast<std::int32_t>(0x{word:08X}u);"
                )
            elif kind == "unsigned int":
                declaration = f"std::uint32_t {item['symbol']} = 0x{word:08X}u;"
            elif kind == "unsigned char":
                declaration = f"std::uint8_t {item['symbol']} = 0x{word:02X}u;"
            else:
                raise ValueError(f"unsupported scalar type {kind}")
            lines.append(f"{declaration} // {item['address']} {item['section']}")
        args.cpp_output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    if args.cpp_arrays_output:
        names = {item.get("symbol") for item in result}
        declarations = [
            "// Generated by tools/audit_native_globals.py from verified",
            f"// initial array ranges of original HP6 PE {digest}.",
            "// Inner labels of overlapping regions remain unresolved.",
            "#include <cstdint>",
            "",
            "struct ShaderSlot { std::uint32_t bytecode; std::uint32_t shader; };",
            "",
        ]
        ranges = []
        for kind, name, count, width in IMAGE_ARRAYS:
            if name not in names:
                raise ValueError(f"symbol absent from original link log: {name}")
            address = int(name[-8:], 16)
            size = count * width
            section, data = bytes_at(image, base, sections, address, size)
            if section != ".data" or data is None:
                raise ValueError(f"array range absent from .data: {name}")
            if any(address < end and start < address + size for start, end in ranges):
                raise ValueError(f"array ranges overlap: {name}")
            ranges.append((address, address + size))
            if any(data):
                if kind not in ("std::uint32_t", "std::int32_t") or width != 4:
                    raise ValueError(f"nonzero array needs an explicit type audit: {name}")
                values = [f"static_cast<std::int32_t>(0x{value:08X}u)"
                          if kind == "std::int32_t" else f"0x{value:08X}u"
                          for (value,) in struct.iter_unpack("<I", data)]
                initializer = "{" + ", ".join(values) + "}"
            else:
                initializer = "{}"
            declarations.append(f"{kind} {name}[{count}] = {initializer}; "
                                f"// 0x{address:08X}, {size:#x} bytes")
        args.cpp_arrays_output.write_text("\n".join(declarations) + "\n", encoding="utf-8")
    if args.cpp_chapters_output:
        def words_at(address, count):
            _, block = bytes_at(image, base, sections, address, count * 4)
            if block is None:
                raise ValueError(f"missing original table at {address:#x}")
            return [value for (value,) in struct.iter_unpack("<I", block)]

        def string_at(address):
            _, block = bytes_at(image, base, sections, address, 128)
            if block is None or b"\0" not in block:
                raise ValueError(f"missing original string at {address:#x}")
            return block.split(b"\0", 1)[0].decode("ascii")

        fixed = [string_at(address) for address in words_at(0x0080A91C, 11)]
        chapter_words = words_at(0x00892300, 30)
        chapters = [
            (chapter_words[index], string_at(chapter_words[index + 1]),
             chapter_words[index + 2])
            for index in range(0, 30, 3)
        ]
        chapter_lines = [
            "// Generated from the original PE by tools/audit_native_globals.py.",
            f"// SHA-256: {digest}",
            "#include <cstdint>",
            "",
            "struct ChapterTableEntry {",
            "    std::int32_t index;",
            "    const char* name;",
            "    std::int32_t next_hint;",
            "};",
            "",
            "const char* PTR_DAT_0080a91c[11] = {",
            *[f"    {json.dumps(value)}," for value in fixed],
            "};",
            "ChapterTableEntry DAT_00892300[10] = {",
            *[f"    {{{index}, {json.dumps(name)}, {hint}}},"
              for index, name, hint in chapters],
            "};",
        ]
        args.cpp_chapters_output.write_text(
            "\n".join(chapter_lines) + "\n", encoding="utf-8"
        )
    if args.cpp_drive_table_output:
        _, pointer_bytes = bytes_at(image, base, sections, 0x008513E0, 26 * 4)
        if pointer_bytes is None:
            raise ValueError("missing original drive pointer table")
        pointers = [value for (value,) in struct.iter_unpack("<I", pointer_bytes)]
        record_base = 0x00892BE8
        record_size = 20
        expected = {record_base + i * record_size for i in range(26)}
        if set(pointers) != expected or len(set(pointers)) != 26:
            raise ValueError("unexpected original drive record layout")
        section, records = bytes_at(image, base, sections, record_base, 26 * record_size)
        if section != ".data" or records is None or any(records):
            raise ValueError("original drive records are missing or not zero")
        drive_lines = [
            "// Generated from verified original PE bytes by tools/audit_native_globals.py.",
            f"// SHA-256: {digest}",
            "#include <cstdint>",
            "#include <cstddef>",
            "",
            "static_assert(sizeof(void*) == 4, \"original drive pointers are x86\");",
            "alignas(4) std::uint8_t native_drive_records[26][20] = {};",
            "extern const std::uint32_t PTR_DAT_008513e0[26] = {",
            *[
                "    static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>("
                f"native_drive_records[{(pointer - record_base) // record_size}])),"
                f" // original slot {index}: 0x{pointer:08X}"
                for index, pointer in enumerate(pointers)
            ],
            "};",
        ]
        args.cpp_drive_table_output.write_text(
            "\n".join(drive_lines) + "\n", encoding="utf-8"
        )
    if args.cpp_heap_overrides_output:
        def heap_words(address, count):
            section, block = bytes_at(image, base, sections, address, count * 4)
            if section != ".data" or block is None:
                raise ValueError(f"missing original heap table at {address:#x}")
            return [value for (value,) in struct.iter_unpack("<I", block)]

        def heap_string(address):
            section, block = bytes_at(image, base, sections, address, 32)
            if block is None or b"\0" not in block:
                raise ValueError(f"missing original heap string at {address:#x}")
            return block.split(b"\0", 1)[0].decode("ascii")

        size_records = heap_words(0x00866C24, 4)
        flag_records = heap_words(0x00866C34, 4)
        name_records = heap_words(0x00866C54, 6)
        pointer_slots = heap_words(0x008796B0, 8)
        if pointer_slots != [2, 0x00866C24, 2, 0x00866C34,
                             2, 0x00866C44, 2, 0x00866C54]:
            raise ValueError("unexpected original heap override pointers")
        names = [heap_string(name_records[index]) for index in (1, 4)]
        if names != ["RenderHeap", "SceneHeap"]:
            raise ValueError("unexpected original heap override names")
        if name_records[2] != 0 or name_records[5] != 0:
            raise ValueError("unexpected heap name record tails")
        heap_lines = [
            "// Generated from verified original PE bytes by tools/audit_native_globals.py.",
            f"// SHA-256: {digest}",
            "#include <cstdint>",
            "#include <cstddef>",
            "",
            "struct HeapWordOverride { std::uint32_t index, value; };",
            "struct HeapNameOverride { std::uint32_t index; const char* name; std::uint32_t reserved; };",
            "static_assert(sizeof(HeapWordOverride) == 8);",
            "static_assert(sizeof(HeapNameOverride) == 12, \"original x86 layout\");",
            "HeapWordOverride native_heap_size_overrides[2] = {",
            f"    {{{size_records[0]}u, 0x{size_records[1]:08X}u}},",
            f"    {{{size_records[2]}u, 0x{size_records[3]:08X}u}},",
            "};",
            "HeapWordOverride native_heap_flag_overrides[2] = {",
            f"    {{{flag_records[0]}u, 0x{flag_records[1]:08X}u}},",
            f"    {{{flag_records[2]}u, 0x{flag_records[3]:08X}u}},",
            "};",
            "HeapNameOverride native_heap_name_overrides[2] = {",
            f"    {{{name_records[0]}u, {json.dumps(names[0])}, 0u}},",
            f"    {{{name_records[3]}u, {json.dumps(names[1])}, 0u}},",
            "};",
            "std::uint8_t* PTR_DAT_008796b4 = reinterpret_cast<std::uint8_t*>(native_heap_size_overrides);",
            "std::uint8_t* PTR_DAT_008796bc = reinterpret_cast<std::uint8_t*>(native_heap_flag_overrides);",
            "std::uint8_t* PTR_DAT_008796cc = reinterpret_cast<std::uint8_t*>(native_heap_name_overrides);",
        ]
        args.cpp_heap_overrides_output.write_text(
            "\n".join(heap_lines) + "\n", encoding="utf-8"
        )
    if args.json:
        print(json.dumps(report, indent=2, ensure_ascii=False))
    else:
        print("binary_sha256", report["binary_sha256"])
        print("pe_entry", report["pe_entry"])
        print("symbol_counts", summary)
        print("section_counts", dict(collections.Counter(
            item.get("section") for item in result if item["kind"] != "function"
        )))
        for item in result:
            if item["kind"] == "function":
                print("unresolved_function", item["symbol"])


if __name__ == "__main__":
    main()
