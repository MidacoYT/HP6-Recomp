import json
from pathlib import Path
from datetime import datetime, timezone

state_path = Path("reports/pipeline/state.json")
s = json.loads(state_path.read_text(encoding="utf-8"))
funcs = s.setdefault("functions", {})

now = datetime.now(timezone.utc).isoformat()

updates = {
    "00401000": ("hp6::math::crt_math_wrapper", "src/math/Scalar.cpp", "float_ops_015"),
    "00401270": ("hp6::math::approx_transcendental_1", "src/math/Scalar.cpp", "float_ops_015"),
    "004013a0": ("hp6::math::approx_transcendental_2", "src/math/Scalar.cpp", "float_ops_015"),
    "00401da0": ("hp6::math::range_check", "src/math/Scalar.cpp", "float_ops_015"),
    "00403190": ("hp6::math::three_param_float", "src/math/Scalar.cpp", "float_ops_015"),
}

for addr, (symbol, source, batch) in updates.items():
    entry = funcs.setdefault(addr, {})
    entry.update({
        "batch": batch,
        "export": f".ghidra-exports/{addr}.json",
        "integration": "integrated",
        "build": "passed",
        "cpp_symbol": symbol,
        "source": source,
        "updated_at": now,
        "integration_note": "ReAgent inaccessible (limite d'utilisation) ; fonction integree manuellement a partir du decompilage Ghidra ; dependances globales externes declarees (0x006f519c, 0x007db234, etc.) avec valeurs par défaut ; build Win32 et smoke tests passes ; equivalencia binaire non verifiee.",
        "equivalence": "unverified",
    })

state_path.write_text(json.dumps(s, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
print(f"Mise à jour de {len(updates)} fonctions dans state.json")
