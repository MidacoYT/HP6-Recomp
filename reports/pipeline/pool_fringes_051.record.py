import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b1815', '006b2dde', '006b45ae', '006b1a59')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'pool_fringes_051',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'pool_fringes_051'
d['batches']['pool_fringes_051'] = {
    'addresses': list(addresses),
    'status': 'integrated',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 051 (Oct 17): the plain create-and-wait wrapper, '
            'the FIFO-specific removal completing the intrusive queue '
            'family, the wrapper state-block reset and the ";"-separated '
            'search path splitter; asm-arbitrated, covered in '
            'pool_wait_smoke.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b1815', 'create_and_wait_plain_node (hp6::runtime)',
     'src/runtime/PoolWait.cpp',
     'pool_fringes_051: creates the plain node (006b2527, no attach '
     'words), waits (006b22c3) and returns the completed pair itself '
     '(006b2359) - unlike 006b17e5 which answers a boolean. Funnel '
     'verified: PUSH 0; PUSH [EBP+0xc]; PUSH [EBP+8].'),
    ('006b2dde', 'queue_remove_006b2dde (hp6::runtime)',
     'src/runtime/ListQueue.cpp',
     'pool_fringes_051: thiscall (RET 8) removal of one specific node '
     'from the FIFO { head +0, tail +4, count +8, link at node+0 }: head '
     'node handled inline, otherwise the search restarts at the supplied '
     'previous hint (the byte at head+0) or at the head and restarts only '
     'while [previous] is non-zero (never dereferences null), unlinks '
     'prev.next = node.next, repairs the tail and zeroes the node link. '
     'Completes the 006db315/006db335 queue family.'),
    ('006b45ae', 'reset_wrapper_block_006b45ae (hp6::runtime)',
     'src/runtime/WrapperStubs.cpp',
     'pool_fringes_051: fastcall (block in ECX, bare RET). Byte-exact '
     'state reset: OR [ECX],-1; OR [ECX+4],-1; dwords +8/+0xc/+0x10 and '
     'bytes +0x14/+0x15 zeroed.'),
    ('006b1a59', 'set_game_search_paths (hp6::runtime)',
     'src/runtime/PoolPath.cpp',
     'pool_fringes_051: clears the array (root+0x88, DAT_00851390 << 2 '
     'bytes) and the buffer (root+0x8c, DAT_00851394 bytes) through '
     'clear_buffer (006b3b0e), copies the list string, parks its address '
     'at array[0], then splits at the CRT strchr thunk (006b9386): each '
     'semicolon becomes a terminator with the preceding slash removed and '
     'the next token stored at the following slot; the final entry receives '
     'the same trailing-slash treatment. The count globals are read, '
     'never written.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
