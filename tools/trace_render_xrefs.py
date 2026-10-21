"""Read-only xrefs for the render dispatch trail in the local Ghidra project."""
from pathlib import Path
import argparse

import pyghidra
import yaml


CONFIG = yaml.safe_load(Path("ghidra-bridge.yaml").read_text(encoding="utf-8"))["ghidra"]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("addresses", nargs="*", type=lambda value: int(value, 16),
                    help="hex addresses; defaults to the known render trail")
parser.add_argument("--words", type=int, default=0,
                    help="also inspect this many 4-byte words after each address")
parser.add_argument("--dump", action="store_true", help="show each word's pointer target")
args = parser.parse_args()
targets = args.addresses or (0x00648730, 0x010120D0, 0x00648630,
                             0x0056BAA0, 0x005DB240, 0x00D63E70)

pyghidra.start(install_dir=Path(CONFIG["install_dir"]))
project = pyghidra.open_project(Path(CONFIG["project_dir"]), CONFIG["project_name"])
program, consumer = pyghidra.consume_program(project, "/" + CONFIG["program_name"])
try:
    space = program.getAddressFactory().getDefaultAddressSpace()
    refs = program.getReferenceManager()
    functions = program.getFunctionManager()
    memory = program.getMemory()
    for value in targets:
        for delta in range(0, 4 * (args.words or 1), 4):
            address = space.getAddress(value + delta)
            found = list(refs.getReferencesTo(address))
            if args.words and not found and not args.dump:
                continue
            if args.dump:
                target_value = memory.getInt(address) & 0xffffffff
                target_address = space.getAddress(target_value)
                target = functions.getFunctionAt(target_address)
                print(f"{value + delta:08x}: {target_value:08x} "
                      f"{target.getName() if target else ''}")
            else:
                print(f"{value + delta:08x}:")
            for ref in found:
                source = ref.getFromAddress()
                function = functions.getFunctionContaining(source)
                name = function.getName() if function else "<data>"
                print(f"  {source} {ref.getReferenceType()} {name}")
finally:
    program.release(consumer)
    project.close()
