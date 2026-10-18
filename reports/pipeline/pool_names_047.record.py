import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b2fbd', '006b289e', '006b2990', '006b28d4')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'pool_names_047',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'pool_names_047'
d['batches']['pool_names_047'] = {
    'addresses': list(addresses),
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 047 (Oct 17 LLM window). Bounded path node '
            'constructor 006b2fbd and the three volume-aware creators '
            '006b289e/006b2990/006b28d4 (engine = find_volume_by_name).',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b2fbd', 'construct_bounded_path_node (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_names_047: thiscall constructor; 006b20e8 attach, vtable '
     'PTR_FUN_00715ab0, 006b1968 path attachment, +0x24 = 0 then '
     '+0x20 = param_2, and 006b2199 scheduling LAST (order preserved from '
     'the decompile).'),
    ('006b289e', 'create_named_node_a_from_record (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_names_047: cdecl creator; 006b1d53(0x40) slot pull, engine = '
     '006b25b8(path) (may be null), 006b2ef3 (path, mode, words, engine). '
     'Null slot path faults at [null+0xc] as in the binary. This ties the '
     'named-node family to the volume registry: the engine is a volume '
     'record of the root + 0x7c list.'),
    ('006b2990', 'create_bounded_path_node_from_record (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_names_047: cdecl creator; 006b1d53(0x40) slot pull, engine = '
     '006b25b8(path), 006b2fbd (path, param_2, mode, words, engine); '
     'returns slot word +0xc; no empty-pool guard (same fault path).'),
    ('006b28d4', 'create_named_node_b_from_volume (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_names_047: cdecl; without a colon the engine is looked up over '
     'the game path (two dereferences from root + 0x88); unless the path '
     'starts with a UNC double backslash, the volume name getter (slot '
     '+0x30, thiscall ECX-only, no stack push in the x86) feeds a '
     '260-byte [EBP-0x104] buffer: name copied with its NUL, then the '
     'path appended over the name terminator including its NUL '
     '(inline strlen counted the NUL, REP MOVSD/MOVSB tail) - exactly '
     'strcpy+strcat semantics, no zeroing. With a colon the engine comes '
     'from the path. 006b2f40 builds the node. The +0x30 slot callee '
     'always cleans one stack word (RET 4, evidenced by 006b25b8 '
     'balance); the reconstruction models the ECX-only call with an '
     'explicit dummy argument so the generated stack stays balanced - '
     'the binary instead lets the callee consume four bytes of its '
     'EBP-relative caller frame, which is behavior-equivalent. Null slot '
     'path faults as in the binary.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
