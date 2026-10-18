import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006db315', '006db335')
for a in addresses:
    entry = f[a]
    entry['rejected_candidate'] = None
    entry.pop('rejected_candidate', None)
d['batches']['pool_leaves2_039'] = {
    **d['batches']['pool_leaves2_039'],
    'status': 'integrated',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': d['batches']['pool_leaves2_039'].get('note', '')
            + ' Completed manually (Oct 17): ReAgent candidates for '
              '006db315/006db335 failed validation, so the two functions '
              'were integrated from the exports; the seven earlier '
              'addresses came through manual lots 043-045.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006db315', 'queue_push_006db315 (hp6::runtime)',
     'src/runtime/ListQueue.cpp',
     'pool_leaves2_039 completion: thiscall callee-clean (RET 4); appends '
     'a node to the intrusive FIFO { head +0, tail +4, count +8 } with the '
     'next-pointer at node +0: node link zeroed, count bumped, tail = node, '
     'then head = node on an empty queue or the old tail links to it. '
     'ReAgent candidate failed its own validation; asm-arbitrated manual '
     'integration.'),
    ('006db335', 'queue_pop_006db335 (hp6::runtime)',
     'src/runtime/ListQueue.cpp',
     'pool_leaves2_039 completion: fastcall ECX-only (RET, no stack '
     'argument). Detaches the head: absent = no-op; a single-node queue '
     'clears head and tail; otherwise head = head.next, then count-- and '
     'the popped node link is zeroed. ReAgent candidate failed its own '
     'validation; asm-arbitrated manual integration.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
