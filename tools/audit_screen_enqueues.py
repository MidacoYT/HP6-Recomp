"""Extract the 24 distinct screen enqueue bodies from the verified PE."""

import csv
import hashlib
import re
from pathlib import Path

import capstone

from audit_native_globals import bytes_at, read_pe
from disasm_original import UNPROTECTED_SHA256

ROOT = Path(__file__).resolve().parent.parent
PE = Path(r"D:\HP\hp6ss\build\sdk-lab\pc\hp6.exe")
image, base, _, sections = read_pe(PE)
if hashlib.sha256(image).hexdigest() != UNPROTECTED_SHA256:
    raise ValueError("unexpected unprotected PE hash")
decoder = capstone.Cs(capstone.CS_ARCH_X86, capstone.CS_MODE_32)

with (ROOT / "architecture/screen_task_table.csv").open(newline="", encoding="utf-8") as handle:
    entries = list(csv.DictReader(handle))
addresses = sorted({int(entry["activate_address"], 16) for entry in entries})

for index, address in enumerate(addresses):
    end = addresses[index + 1] if index + 1 < len(addresses) else address + 0x70
    _, code = bytes_at(image, base, sections, address, end - address)
    instructions = list(decoder.disasm(code, address))
    ret = next((i for i in instructions if i.mnemonic == "ret"), None)
    if ret is None:
        raise ValueError(f"no return at {address:08x}")
    instructions = [i for i in instructions if i.address <= ret.address]
    names = [entry["screen"] for entry in entries
             if int(entry["activate_address"], 16) == address]
    immediate_pushes = [i.op_str for i in instructions if i.mnemonic == "push"]
    absolute_stores = [(i.address, i.op_str) for i in instructions
                       if i.mnemonic == "mov" and re.search(r"\[0x[0-9a-f]{6,}\]", i.op_str)]
    object_stores = [(i.address, i.op_str) for i in instructions
                     if i.mnemonic == "mov" and ("[eax" in i.op_str or "[ecx" in i.op_str)]
    print(f"{address:08x} {', '.join(names)}")
    print(f"  pushes: {immediate_pushes}")
    print(f"  absolute stores: {absolute_stores}")
    print(f"  object stores: {object_stores}")
