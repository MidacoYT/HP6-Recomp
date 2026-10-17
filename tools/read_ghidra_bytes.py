"""Read bytes from the existing Ghidra project without changing its exports."""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path

import pyghidra
from jpype import JArray, JByte
import yaml


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("address", type=lambda value: int(value, 16))
    parser.add_argument("count", type=int)
    parser.add_argument("--config", type=Path, default=Path("ghidra-bridge.yaml"))
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    if not 0 < args.count <= 4096:
        parser.error("count must be between 1 and 4096")

    config = yaml.safe_load(args.config.read_text(encoding="utf-8"))["ghidra"]
    pyghidra.start(install_dir=Path(config["install_dir"]))
    project = pyghidra.open_project(Path(config["project_dir"]), config["project_name"])
    program, consumer = pyghidra.consume_program(project, "/" + config["program_name"])
    try:
        address = program.getAddressFactory().getDefaultAddressSpace().getAddress(args.address)
        data = JArray(JByte)(args.count)
        count = program.getMemory().getBytes(address, data)
        if count != args.count:
            raise RuntimeError(f"Only {count} of {args.count} requested bytes were available")
        extracted = bytes(value & 0xff for value in data[:count])
        if args.output:
            args.output.parent.mkdir(parents=True, exist_ok=True)
            args.output.write_bytes(extracted)
            print(f"{args.address:08x}: {count} bytes -> {args.output}, "
                  f"SHA256 {hashlib.sha256(extracted).hexdigest()}")
        else:
            print(f"{args.address:08x}: {extracted.hex(' ')}")
    finally:
        program.release(consumer)
        project.close()


if __name__ == "__main__":
    main()
