import json
from pathlib import Path
from datetime import datetime, timezone

state_path = Path("reports/pipeline/state.json")
s = json.loads(state_path.read_text(encoding="utf-8"))
funcs = s.setdefault("functions", {})

now = datetime.now(timezone.utc).isoformat()
funcs["006b938c"] = {
    "batch": "external_crt",
    "export": ".ghidra-exports/006b938c.json",
    "ghidra_signature": "int __cdecl sprintf(char * _Dest, char * _Format, ...)",
    "reagent_exit_code": 0,
    "reagent_verdict": "EXTERNAL",
    "reagent_validation": "CRT",
    "reagent_run_id": "external-crt-sprintf",
    "error": None,
    "updated_at": now,
    "integration": "integrated",
    "cpp_symbol": "std::sprintf (CRT external)",
    "source": None,
    "build": "n/a",
    "integration_note": "Fonction C runtime standard (sprintf) identifiée via Ghidra ; pas de reconstruction C++ requise, appel direct vers <cstdio>.",
    "equivalence": "unverified",
}

state_path.write_text(json.dumps(s, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
print("006b938c (sprintf) ajoute comme dependance externe integratee")
