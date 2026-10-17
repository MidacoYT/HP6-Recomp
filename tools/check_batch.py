import json
from pathlib import Path
import sys

batch_name = sys.argv[1] if len(sys.argv) > 1 else "float_ops_015"

s = json.loads(Path("reports/pipeline/state.json").read_text(encoding="utf-8"))
batches = s.get("batches", {})
if batch_name not in batches:
    print(f"Batch '{batch_name}' non trouve")
    sys.exit(1)

print(json.dumps(batches[batch_name], indent=2))

funcs = s.get("functions", {})
for a in batches[batch_name]["addresses"]:
    e = funcs.get(a, {})
    print(f"{a}: integration={e.get('integration')}, verdict={e.get('reagent_verdict')}, build={e.get('build')}")

