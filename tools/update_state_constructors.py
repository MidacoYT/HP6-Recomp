import json
from pathlib import Path
from datetime import datetime, timezone

state_path = Path("reports/pipeline/state.json")
s = json.loads(state_path.read_text(encoding="utf-8"))
funcs = s.setdefault("functions", {})

now = datetime.now(timezone.utc).isoformat()

updates = {
    "006d6ab6": ("hp6::projection::invoke_global_callback", "src/projection/RecordDecode.cpp"),
    "006d793d": ("hp6::projection::construct_variant_80_record", "src/projection/RecordDecode.cpp"),
    "006d7980": ("hp6::projection::create_clut_element", "src/projection/RecordDecode.cpp"),
    "006d79fa": ("hp6::projection::create_shp_record_variant_20", "src/projection/RecordDecode.cpp"),
    "006d7d9c": ("hp6::projection::create_shp_record_variant_02", "src/projection/RecordDecode.cpp"),
}

for addr, (symbol, source) in updates.items():
    entry = funcs.setdefault(addr, {})
    entry.update({
        "batch": "projection_constructors_014",
        "export": f".ghidra-exports/{addr}.json",
        "integration": "integrated",
        "build": "passed",
        "cpp_symbol": symbol,
        "source": source,
        "updated_at": now,
        "integration_note": "ReAgent inaccessible (limite d'utilisation Codex) ; fonction integree manuellement a partir du decompilage Ghidra et de l'assembleur exporte ; build Win32 et smoke tests passes ; equivalencia binaire non verifiee.",
        "equivalence": "unverified",
    })

state_path.write_text(json.dumps(s, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
print(f"Mise à jour de {len(updates)} fonctions dans state.json")
