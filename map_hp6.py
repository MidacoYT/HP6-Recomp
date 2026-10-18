
import json
import csv
import re
from pathlib import Path
from collections import Counter, defaultdict

ROOT = Path(".ghidra-exports")
OUT = Path("architecture")
OUT.mkdir(exist_ok=True)

def load(name):
    with (ROOT / name).open("r", encoding="utf-8-sig") as f:
        return json.load(f)

def addr(value):
    if isinstance(value, str):
        value = value.lower().replace("0x", "")
        if re.fullmatch(r"[0-9a-f]+", value):
            return value.zfill(8)
    return None

def write_csv(name, rows, columns):
    with (OUT / name).open("w", newline="", encoding="utf-8-sig") as f:
        writer = csv.DictWriter(f, fieldnames=columns, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)

index = load("_index.json")
structs = load("_structs.json")
vtables = load("_vtables.json")
strings = load("_strings.json")
string_refs = load("_string_refs_by_function.json")
global_refs = load("_global_refs_by_function.json")

functions = []
named = []
groups = defaultdict(list)

def classify(name):
    low = name.lower()
    if re.fullmatch(r"(fun|sub|thunk|lab|loc)_[0-9a-f]+", low):
        return "unknown"
    if "::" in name:
        return "qualified"
    return "named"

for address, item in index.items():
    name = str(item.get("name") or f"FUN_{address}")
    a = addr(address) or address
    status = classify(name)

    row = {
        "address": a,
        "name": name,
        "status": status,
        "callers": item.get("num_callers", 0),
        "callees": item.get("num_callees", 0),
        "string_refs": len(string_refs.get(a, [])),
        "global_refs": len(global_refs.get(a, [])),
    }
    functions.append(row)

    if status != "unknown":
        named.append(row)

    if "::" in name:
        owner = name.rsplit("::", 1)[0]
        groups[owner].append(row)

write_csv(
    "functions.csv", functions,
    ["address", "name", "status", "callers", "callees",
     "string_refs", "global_refs"]
)

write_csv(
    "named_functions.csv", named,
    ["address", "name", "status", "callers", "callees",
     "string_refs", "global_refs"]
)

class_rows = []
for owner, methods in sorted(groups.items()):
    class_rows.append({
        "owner": owner,
        "method_count": len(methods),
        "addresses": ", ".join(m["address"] for m in methods[:20]),
        "evidence": "qualified_symbol",
    })

write_csv(
    "class_candidates.csv", class_rows,
    ["owner", "method_count", "addresses", "evidence"]
)

struct_rows = []
for name, item in structs.items():
    fields = item.get("fields") or []
    struct_rows.append({
        "name": name,
        "path": item.get("path", ""),
        "size": item.get("size", ""),
        "field_count": len(fields),
    })

write_csv(
    "structures.csv", struct_rows,
    ["name", "path", "size", "field_count"]
)

vtable_rows = []
for name, item in vtables.items():
    vtable_rows.append({
        "symbol": item.get("symbol", name),
        "class_name": item.get("class_name") or "",
        "address": item.get("address") or "",
        "entry_count": item.get("entry_count") or 0,
    })

write_csv(
    "vtables.csv", vtable_rows,
    ["symbol", "class_name", "address", "entry_count"]
)

# Chaînes susceptibles de fournir des noms techniques.
# Ce sont des indices, pas des classes confirmées.
patterns = (
    "renderware", "eventhandler", "animation",
    "character", "camera", "shader", "material",
    "physics", "spell", "memory", "entity",
    "resource", "texture", "script", "audio",
)

string_rows = []
for address, item in strings.items():
    value = str(item.get("value") or "")
    if any(word in value.lower() for word in patterns):
        string_rows.append({
            "address": address,
            "value": value[:500],
            "length": item.get("length", ""),
        })

write_csv(
    "technical_strings.csv", string_rows,
    ["address", "value", "length"]
)

summary = {
    "function_count": len(functions),
    "named_function_count": len(named),
    "qualified_symbol_count": sum(
        1 for f in functions if f["status"] == "qualified"
    ),
    "qualified_owner_candidates": len(groups),
    "structure_count": len(structs),
    "vtable_count": len(vtables),
    "string_count": len(strings),
    "technical_string_count": len(string_rows),
    "string_reference_key_count": len(string_refs),
    "global_reference_key_count": len(global_refs),
}

with (OUT / "summary.json").open("w", encoding="utf-8") as f:
    json.dump(summary, f, indent=2, ensure_ascii=False)

print("\n=== HP6 ARCHITECTURE ===")
for key, value in summary.items():
    print(f"{key}: {value}")

print("\nTop des propriétaires de méthodes qualifiées :")
for owner, methods in sorted(
    groups.items(), key=lambda pair: -len(pair[1])
)[:20]:
    print(f"  {owner}: {len(methods)}")

print("\nRapports enregistrés dans :", OUT.resolve())
