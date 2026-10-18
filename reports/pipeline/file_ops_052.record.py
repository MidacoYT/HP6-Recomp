"""Lot 052 registration: reserved file operators (mixed_deps_060 resolved).

Quoting-safe: all note strings live here, written through json.dump and
subprocess lists (no shell echo with embedded colons or quotes).
Run with .\\.venv\\Scripts\\python.exe reports/pipeline/file_ops_052.record.py
"""
import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
new_addresses = (
    '006b329a', '006b24f2', '006b17b7',
    '006d9f47', '006da0c0', '006da462',
)
wrapper_addresses = (
    '006da081', '006da10c', '006da14e', '006da4b1', '006da4f3',
)
addresses = new_addresses + wrapper_addresses
for a in new_addresses:
    if a not in f:
        f[a] = {'batch': 'file_ops_052',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
for a in addresses:
    f[a]['batch'] = 'file_ops_052'
superseded = d['batches'].get('mixed_deps_060')
if superseded is not None:
    superseded['addresses'] = []
    superseded['status'] = 'superseded'
    superseded['updated_at'] = datetime.now(timezone.utc).isoformat()
    superseded['note'] = ('The five blocked reserved-file wrappers moved to '
                          'the manual lot file_ops_052 once the token versus '
                          'record chaining was resolved; their external '
                          'dependencies 006b2553 and 006b25b8 are integrated.')
d['batches']['file_ops_052'] = {
    'addresses': list(addresses),
    'status': 'integrated',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 052 (Oct 18): the reserved-callback file operators. '
            'The binary derefs word(record+0xc) for the engine slot, so the '
            'workers chain on the pair low returned by 006b175b, while the '
            'creators return the packed token; the probe constructor keeps '
            'the raw vtable 0x715af0. Covered in pool_wait_smoke.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b329a', 'construct_probe_node_00715af0 (hp6::runtime)',
     'src/runtime/PoolList.cpp',
     'file_ops_052: thiscall probe constructor (callee-clean RET 0x1c). '
     'Attaches the engine first (006b20e8), stores the record at +0x14, '
     'writes the two CDQ sign pairs +0x28/+0x2c and +0x20/+0x24 '
     'interleaved with the raw vtable 0x715af0, the unsigned field +0x30 '
     'last, then schedules through 006b2199.'),
    ('006b24f2', 'create_probe_node_from_record (hp6::runtime)',
     'src/runtime/PoolList.cpp',
     'file_ops_052: cdecl creator. Pulls a 0x40 slot (006b1d53) and calls '
     '006b329a with the decompiled funnel (slot, param_1, param_4, '
     'param_2, param_3, param_5, param_6, word(param_1+0xc)); no '
     'empty-pool guard; answers the node token at slot+0xc.'),
    ('006b17b7', 'create_and_wait_probe_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'file_ops_052: cdecl create-and-wait over the probe creator; waits '
     '(006b22c3) and returns the completed pair itself (006b2359), '
     'unlike 006b17e5 which answers a boolean.'),
    ('006d9f47', 'open_file_reserved (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: reserved worker of 006da081. Chains the bounded path '
     'record pair low (006b175b, field 1, mode reserve), the plain pair '
     '(006b1815, mode reserve-1), then hands the 18-dword hlsfile '
     'descriptor (6, 0xac, self-pointer, 5, 0x71b358 string, 7, ctx+4, '
     '1, path, and trailing flag words read from ctx[4]) to the '
     'DAT_00851384 vtable slot 0 thiscall; on a non-zero answer queues '
     '006b1783(answer, record 2) before the final 006b17e5, parks '
     'record 2 into ctx[2] and returns the answer, else waits and '
     'answers 0.'),
    ('006da0c0', 'write_file_reserved (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: reserved worker of 006da10c and 006da14e. Chains the '
     'bounded path record pair low (field 1, mode reserve), queues the '
     'repeat stage 006b1783(0, ctx[1], ctx[2], mode reserve-1), then the '
     'final plain wait 006b17e5 with the answer discarded; answers the '
     'repeat pair low.'),
    ('006da462', 'check_file_reserved (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: reserved worker of 006da4b1 and 006da4f3. Chains the '
     'bounded path record pair low (field 6, mode reserve), queues the '
     'probe stage 006b17b7(0, ctx[1], ctx[2], mode reserve-1), waits '
     'once more and answers whether the probe pair low equals ctx[2] '
     '(SETZ).'),
    ('006da081', 'open_file_from_path (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: cdecl wrapper. Resolves the volume record for the '
     'path (006b25b8), parks the context {path, -, out slot, 0, arg} and '
     'reserves 0x64 through 006b2553; copies the worker result ctx[2] '
     'into the out pointer afterwards, no answer itself.'),
    ('006da10c', 'write_file_flagged (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: cdecl wrapper. Context {path, a, b, 1, c} (the flag '
     'word MOV 0x1) and reserve 0x64 through 006b2553 into '
     '006da0c0; no answer.'),
    ('006da14e', 'write_file_plain (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: 006da10c with the flag word cleared instead '
     '(AND [EBP-8],0 in the exported x86).'),
    ('006da4b1', 'check_file_flagged (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: cdecl wrapper over a four-word context {path, a, b, '
     '1}; answers (worker == 1) through the DEC/NEG/SBB/INC tail idiom.'),
    ('006da4f3', 'check_file_plain (hp6::runtime)',
     'src/runtime/FileOps.cpp',
     'file_ops_052: 006da4b1 with the flag word cleared (AND [EBP-8],0).'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
