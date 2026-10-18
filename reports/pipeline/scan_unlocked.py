import json

info = {}
for line in open('architecture/function_map.jsonl', encoding='utf-8'):
    it = json.loads(line)
    info[it['address']] = it

dependents = {}
for a, it in info.items():
    for c in it.get('callees', []):
        dependents.setdefault(c, []).append(a)

d = json.load(open('reports/pipeline/state.json', encoding='utf-8'))
f = d['functions']
integrated = {a for a, v in f.items() if v.get('integration') == 'integrated'}
known = set(f)

rows = []
for a, it in info.items():
    if a in known or not a.startswith('006b'):
        continue
    callees = it.get('callees', [])
    if all(c in integrated for c in callees):
        rows.append((len(dependents.get(a, [])), a, it.get('decompiled_chars', 0),
                     callees))

rows.sort(reverse=True)
print('unlocked in 006b:', len(rows))
for n, a, sz, callees in rows[:30]:
    print(n, a, sz, callees)
