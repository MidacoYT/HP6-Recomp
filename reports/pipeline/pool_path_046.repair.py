import json

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
# Repair a string-concatenation artifact in the 006b1968 note.
note = f['006b1968']['integration_note']
note = note.replace("006b2f40/006b2fbd>s dependency chain.",
                    "006b2f40/006b2fbd dependency chain.")
f['006b1968']['integration_note'] = note
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('note repaired:', note[-60:])
