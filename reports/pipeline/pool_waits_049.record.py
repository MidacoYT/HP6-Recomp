import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b23a9', '006b17e5', '006b183a', '006b175b', '006b1783',
             '006b2553', '006b54b8')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'pool_waits_049',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'pool_waits_049'
d['batches']['pool_waits_049'] = {
    'addresses': list(addresses),
    'status': 'integrated',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 049 (Oct 17): create-and-wait wrappers of the pool '
            'family plus the reserved wheel callback and the constant-true '
            'wrapper stub; asm-arbitrated, covered by pool_wait_smoke with a '
            'wheel-driven one-shot completion (fire_drain).',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b23a9', 'completed_state (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_waits_049: bare JMP 006b2359 - completed-answer passthrough, '
     'asm-exact.'),
    ('006b17e5', 'create_and_wait_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_waits_049: creates the timed node with no attach words '
     '(006b2456), waits for completion (006b22c3), reads the pair back '
     '(006b2359) and answers 1 exactly when that pair is non-zero. Funnel '
     'verified in the export: PUSH 0; PUSH [EBP+0xc]; PUSH [EBP+8].'),
    ('006b183a', 'create_and_wait_named_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_waits_049: same chain over the volume-lookup named node A '
     '(006b289e) with no attach words.'),
    ('006b175b', 'create_and_wait_bounded_path_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_waits_049: same chain over the volume-lookup bounded path node '
     '(006b2990) with no attach words; returns the 64-bit pair itself '
     '(ADD ESP,0x18 balance verified).'),
    ('006b1783', 'create_and_wait_repeat_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_waits_049: same chain over the repeat creator (006b24b7); each '
     'signed argument reaches the record as a CDQ sign-extended pair '
     '(value, value>>31) at +0x28/+0x2c and +0x24/+0x30.'),
    ('006b2553', 'run_reserved_callback (hp6::runtime)',
     'src/runtime/PoolAccess.cpp',
     'pool_waits_049: runs a cdecl callback (reserve value, argument) under '
     'the lock at frame+0x4c with the reserve word visibly held at +0x7c, '
     'then restores it and signals the auto-reset event at +0x48 '
     '(006d92da). The carrying frame is the record, or the wheel base '
     '(root +0x98) when the record is slot-flagged at +0x80; the record is '
     'bootstrapped first (006b203a) when +4 is clear.'),
    ('006b54b8', 'wrap_true_006b54b8 (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'pool_waits_049: MOV AL,1; RET - constant-true stub with no stack '
     'cleanup (cdecl), consulted by the window handlers of the wrapper '
     'family (8 dependents).'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
