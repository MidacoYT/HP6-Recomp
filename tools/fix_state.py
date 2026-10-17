import json
from pathlib import Path
from datetime import datetime, timezone

state_path = Path("reports/pipeline/state.json")
s = json.loads(state_path.read_text(encoding="utf-8"))
funcs = s.setdefault("functions", {})

updates = {
    "006d760a": ("hp6::projection::estimate_variant_04_size", "src/projection/FormatSize.cpp"),
    "006d768d": ("hp6::projection::estimate_variant_10_size", "src/projection/FormatSize.cpp"),
    "006d778a": ("hp6::projection::decode_variant_04", "src/projection/RecordDecode.cpp"),
    "006d78a8": ("hp6::projection::decode_variant_10", "src/projection/RecordDecode.cpp"),
    "006d822a": ("hp6::projection::clone_variant_02", "src/projection/RecordDecode.cpp"),
}

now = datetime.now(timezone.utc).isoformat()
for addr, (symbol, source) in updates.items():
    entry = funcs.setdefault(addr, {})
    entry.update({
        "integration": "integrated",
        "build": "passed",
        "cpp_symbol": symbol,
        "source": source,
        "updated_at": now,
        "integration_note": "ReAgent candidate blocked (FAIL) but function re-implémentée à partir de l'analyse x86 et des instructions exportées ; build Win32 et smoke tests passés ; équivalence binaire non vérifiée.",
        "equivalence": "unverified",
    })

state_path.write_text(json.dumps(s, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
print(f"Mise à jour de {len(updates)} fonctions dans state.json")
