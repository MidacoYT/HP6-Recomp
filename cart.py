
import re
import idautils

patterns = {
    "C++ classes/namespaces": r"\b[A-Za-z_]\w*(?:::[A-Za-z_]\w*)+",
    "Source files": r"[\w./\\-]+\.(?:cpp|cxx|cc|hpp|h)\b",
    "Windows paths": r"[A-Za-z]:\\[^\r\n\"<>|]+",
}

compiled = {
    name: re.compile(pattern, re.IGNORECASE)
    for name, pattern in patterns.items()
}

results = {name: [] for name in patterns}

for s in idautils.Strings():
    value = str(s)

    for name, regex in compiled.items():
        if regex.search(value):
            results[name].append((s.ea, value))

for name, matches in results.items():
    print("\n" + "=" * 60)
    print(name, "-", len(matches), "matches")
    print("=" * 60)

    for ea, value in matches[:100]:
        print(f"0x{ea:08X} : {value[:250]}")

print("\nRecherche terminee.")
