"""Read the extracted HP6 language-selection chapter without launching the game.

Uses the sibling hp6 STRM reader in the D:\\HP workspace. The report is
evidence for reconstructing menu objects; it does not instantiate them.
"""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import re
import struct
import sys

workspace = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(workspace))
from hp6.hashing import name_hash  # noqa: E402
from hp6.str_format import StrFile  # noqa: E402


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", nargs="?", type=Path,
                        default=workspace / "extract/chapters/languageselect.str")
    args = parser.parse_args()
    bank = StrFile.load(args.source)
    names = {name_hash(name): name for name in bank.strings()}
    assets = {asset.name_hash: asset for asset in bank.assets()}

    def references(asset_name: str) -> list[str]:
        asset = assets[name_hash(asset_name)]
        data = bank.asset_bytes(asset)
        found = []
        for offset in range(0, len(data) - 3, 4):
            target = struct.unpack_from("<I", data, offset)[0]
            if target in assets and target != asset.name_hash:
                found.append(names[target].rsplit("\\", 1)[-1])
        return found

    def children(asset_name: str) -> list[str]:
        data = bank.asset_bytes(assets[name_hash(asset_name)])
        count, start = struct.unpack_from("<2I", data, 0x0c)
        if start + count * 20 > len(data):
            raise ValueError(f"{asset_name}: child table exceeds asset")
        result = []
        for index in range(count):
            target = struct.unpack_from("<I", data, start + index * 20)[0]
            if target not in assets:
                raise ValueError(f"{asset_name}: unknown child hash {target:08x}")
            result.append(names[target].rsplit("\\", 1)[-1])
        return result

    entries = []
    inventory = []
    camera = None
    for asset in bank.assets():
        name = names.get(asset.name_hash, f"{asset.name_hash:08x}")
        short_name = name.rsplit("\\", 1)[-1]
        inventory.append({"name": short_name, "type_hash": f"{asset.type_hash:08x}"})
        if short_name == "CameraLanguageSelectMenu.hybrid":
            data = bank.asset_bytes(asset)
            camera = {
                "basis": list(struct.unpack_from("<12f", data, 0x20)),
                "position": list(struct.unpack_from("<3f", data, 0x50)),
            }
        match = re.fullmatch(r"text_LangaugeSelectMenu(\d+)\.hybrid", short_name)
        if not match or asset.type_hash != 0x06C5B0B9:
            continue
        data = bank.asset_bytes(asset)
        literals = re.findall(rb"[ -~]{4,}", data)
        keys = [value.decode("ascii") for value in literals
                if re.fullmatch(rb"GUI_LANGUAGEMENU\d+", value)]
        index = int(match.group(1))
        if keys != [f"GUI_LANGUAGEMENU{index}"]:
            raise ValueError(f"{short_name}: unexpected localization keys {keys!r}")
        entries.append({
            "index": index,
            "asset": short_name,
            "key": keys[0],
            "position": list(struct.unpack_from("<3f", data, 0x50)),
            "font": next((value.decode("ascii") for value in literals
                          if value.startswith(b"LS_Ascender")), None),
        })
    entries.sort(key=lambda entry: entry["index"])
    if [entry["index"] for entry in entries] != list(range(16)):
        raise ValueError("expected exactly the 16 language menu text assets")
    if camera is None:
        raise ValueError("CameraLanguageSelectMenu.hybrid is missing")
    screen_children = children("ScreenLanguageSelect")
    if screen_children != ["Menu_LanguageSelectMenu.hybrid"] or \
            "CameraLanguageSelectMenu.hybrid" not in references("ScreenLanguageSelect"):
        raise ValueError("screen does not contain the menu and camera references")
    menu_groups = children("Menu_LanguageSelectMenu")
    expected_groups = [f"Group_{index}.hybrid" for index in range(16)]
    if menu_groups != expected_groups:
        raise ValueError(f"unexpected menu group order: {menu_groups!r}")
    groups = []
    for index, group in enumerate(expected_groups):
        group_children = children(group)
        expected = [f"text_LangaugeSelectMenu{index}.hybrid",
                    f"Marker_{index}.hybrid"]
        if group_children != expected:
            raise ValueError(f"{group}: unexpected children {group_children!r}")
        groups.append({"group": group, "children": group_children})
    print(json.dumps({
        "source": str(args.source),
        "sha256": hashlib.sha256(args.source.read_bytes()).hexdigest(),
        "asset_count": len(inventory),
        "camera": camera,
        "entries": entries,
        "screen_children": screen_children,
        "screen_references": references("ScreenLanguageSelect"),
        "groups": groups,
        "structure": [asset["name"] for asset in inventory if asset["name"] in {
            "ScreenLanguageSelect.hybrid", "Menu_LanguageSelectMenu.hybrid",
            "CameraLanguageSelectMenu.hybrid", "LS_Ascender_Uni_211.ffn"}],
    }, indent=2))


if __name__ == "__main__":
    main()
