import json
from pathlib import Path

s = json.loads(Path("reports/pipeline/state.json").read_text(encoding="utf-8"))
funcs = s.get("functions", {})
integrated = sorted(a for a, e in funcs.items() if e.get("integration") == "integrated")
pending = sorted(a for a, e in funcs.items() if e.get("integration") == "pending")
blocked = sorted(a for a, e in funcs.items() if e.get("integration") == "blocked")
print(f"TOTAL: {len(funcs)}")
print(f"INTEGRATED ({len(integrated)}):")
for a in integrated:
    e = funcs[a]
    print(f"  {a} -> {e.get('cpp_symbol','?')} [{e.get('source','?')}] build={e.get('build','?')}")
print(f"PENDING ({len(pending)}): {pending}")
print(f"BLOCKED ({len(blocked)}): {blocked}")
