import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
records = ('006b454a', '006b4555', '006b4560', '006b54bb', '006b54be',
           '006b9730', '006b97bc')
for a in records:
    if a not in f:
        f[a] = {'batch': 'wrap_leaves_048',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'wrap_leaves_048'
d['batches']['wrap_leaves_048'] = {
    'addresses': ['006b454a', '006b4555', '006b4560', '006b54bb', '006b54be',
                  '006b9730', '006b97bc'],
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 048 (Oct 17 LLM window). Byte-arbitrated no-op '
            'methods of the DirectDraw-style wrapper family plus the '
            'memcpy/memset import thunks; multipliers for the 006b4500+ '
            'wrapper cluster (119 + 54 dependents for the thunks).',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
die = 'reports\\pipeline\\pool_path_046.record.py'
entries = [
    ('006b454a', 'wrap_noop_006b454a (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'wrap_leaves_048: this-adjust forwarder (PUSH EBP; MOV EBP,ESP; '
     'MOV ECX,[ECX]; POP EBP; JMP 006b54b2) whose target is the bare '
     'RET 4 no-op: the method writes nothing; one stack word is cleaned.'),
    ('006b4555', 'wrap_noop_006b4555 (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'wrap_leaves_048: same forwarder shape with JMP 006b54b5 (RET 4 '
     'no-op); one stack word cleaned.'),
    ('006b4560', 'wrap_noop_006b4560 (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'wrap_leaves_048: same forwarder shape with JMP 006b54af (RET 8 '
     'no-op); two stack words cleaned.'),
    ('006b54bb', 'wrap_noop_006b54bb (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'wrap_leaves_048: direct bare RET 4 no-op; also the tail-jump target '
     'of the 006b44ee adjust-forwarder.'),
    ('006b54be', 'wrap_noop_006b54be (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'wrap_leaves_048: bare RET no-op called from 006b6306 with `this` '
     'left in ECX (cdecl from the caller side). Interactive gate at '
     '[this+0x6a].'),
    ('006b9730', 'std::memcpy (CRT external)',
     'src/runtime/PoolPath.cpp',
     'wrap_leaves_048: memcpy import thunk (JMP dword ptr [0x00713278], '
     'byte-verified in hp6.exe); 119 dependents use it through the CRT '
     'dispatch family already recorded in copy_dispatch_045.'),
    ('006b97bc', 'std::memset (CRT external)',
     'src/runtime/PoolPath.cpp',
     'wrap_leaves_048: memset import thunk (JMP dword ptr '
     '[0x007132a4], byte-verified in hp6.exe); sibling of the '
     'copy_dispatch_045 slot table.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
