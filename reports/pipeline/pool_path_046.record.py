import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b1968', '006b2ef3', '006b2f40', '006b2482', '006b2527')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'pool_path_046',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'pool_path_046'
d['batches']['pool_path_046'] = {
    'addresses': list(addresses),
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 046 (Oct 17 LLM window). Node path attachment '
            '006b1968 (loader error record, DAT_00851384 vtable slot +0 '
            'thiscall with callee-cleaned stack args, std::memcpy copy) '
            'with the two named-node constructors 006b2ef3/006b2f40 and '
            'the record-fed signed/plain creators 006b2482/006b2527.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b1968', 'attach_node_path (hp6::runtime)', 'src/runtime/PoolPath.cpp',
     'pool_path_046: thiscall node<->path argument (node ECX, path stack). '
     'Null path falls back to the empty string literal (binary 0x007278d3 '
     'verified as a bare NUL in hp6.exe). Inline strlen; 12-dword error '
     'record on the stack ([EBP-0x30..-0x4]: type 6, line 0x1cc, '
     'sub-record {5, filesys.cpp literal at 0x715a30}, flag words 1/1/4/0). '
     'The object DAT_00851384 (vtable slot +0, thiscall with callee-cleaned '
     'stack args length then record) returns a buffer stored at node+0x30; '
     'std::memcpy copies strlen+1 bytes (006b3b04 dispatch slot 008516ac). '
     'This unblocked 006b2ef3/006b2f40/006b2fbd>' 's dependency chain.'),
    ('006b2ef3', 'construct_named_node_00715a90 (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_path_046: thiscall constructor; 006b20e8 attach, vtable '
     'PTR_FUN_00715a90, 006b1968 path attachment, 006b2199 scheduling.'),
    ('006b2f40', 'construct_named_node_00715aa0 (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_path_046: thiscall constructor identical in writer shape to '
     '006b2ef3 but with vtable PTR_FUN_00715aa0.'),
    ('006b2482', 'create_signed_node_from_record (hp6::runtime)',
     'src/runtime/PoolList.cpp',
     'pool_path_046: cdecl creator; 006b1d53(0x40) slot pull then '
     '006b3094 with the funnel (record, bound_low=p4, bound_high=p2, '
     'tag=p3, mode=p5, words=p6, record+0xc); returns slot word +0xc. NO '
     'empty-pool guard: the exported x86 falls through the null branch '
     'into MOV EAX,[EAX+0xc] and faults - mirrored, no synthesized '
     'handling. Same pattern applied to 006b2456/24b7 (their earlier '
     'synthesized null-return guards were removed as deviations).'),
    ('006b2527', 'create_plain_node_from_record (hp6::runtime)',
     'src/runtime/PoolList.cpp',
     'pool_path_046: cdecl creator; 006b1d53(0x40) slot pull then '
     '006b335c (record, mode=p2, words=p3, record+0xc); returns slot word '
     '+0xc, no empty-pool guard (same fault path as the binary).'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
