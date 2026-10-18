
import json
from pathlib import Path

root = Path(".ghidra-exports")

files = [
    "_index.json",
    "_structs.json",
    "_vtables.json",
    "_strings.json",
    "_string_refs_by_function.json",
    "_global_refs_by_function.json",
]

def describe(value, depth=0):
    if isinstance(value, dict):
        print("Type: objet | Entrées:", len(value))
        print("Clés:", list(value.keys())[:12])
        for key, item in list(value.items())[:2]:
            print(f"  {key}: {type(item).__name__}")
            if isinstance(item, dict):
                print("    Champs:", list(item.keys())[:15])
            elif isinstance(item, list):
                print("    Éléments:", len(item))
                if item and isinstance(item[0], dict):
                    print("    Champs du premier:", list(item[0].keys())[:15])
    elif isinstance(value, list):
        print("Type: liste | Éléments:", len(value))
        if value:
            print("Premier élément:", str(value[0])[:350])
    else:
        print("Type:", type(value).__name__)
        print("Valeur:", str(value)[:350])

for name in files:
    path = root / name
    print("\n" + "=" * 55)
    print(name)

    if not path.exists():
        print("INTROUVABLE")
        continue

    try:
        with path.open("r", encoding="utf-8-sig") as f:
            data = json.load(f)
        describe(data)
    except Exception as exc:
        print("ERREUR:", exc)

print("\n" + "=" * 55)
print("EXEMPLE DE FONCTION")
functions = sorted(root.glob("[0-9a-fA-F]*.json"))
if functions:
    path = functions[0]
    print("Fichier:", path.name)
    try:
        with path.open("r", encoding="utf-8-sig") as f:
            describe(json.load(f))
    except Exception as exc:
        print("ERREUR:", exc)
