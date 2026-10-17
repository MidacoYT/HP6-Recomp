import json
from pathlib import Path
from datetime import datetime, timezone

state_path = Path("reports/pipeline/state.json")
s = json.loads(state_path.read_text(encoding="utf-8"))
funcs = s.setdefault("functions", {})

now = datetime.now(timezone.utc).isoformat()

updates = {
    "005d16f0": ("hp6::math::cross_xyz", "src/math/Vector.cpp", "vector_core_001"),
    "005e8b50": ("hp6::math::is_any_nan_xyz", "src/math/Scalar.cpp", "vector_ops_002"),
    "0068c0a0": ("hp6::math::slope_greater", "src/math/Scalar.cpp", "vector_ops_002"),
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
        "integration_note": "Fonction differee auparavant ; integree manuellement a partir de l'assembleur Ghidra exporte ; build Win32 et smoke tests passes ; equivalencia binaire non verifiee.",
        "equivalence": "unverified",
    })

state_path.write_text(json.dumps(s, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
print(f"Mise à jour de {len(updates)} fonctions dans state.json")
