import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b3094', '006b335c', '006b3b04', '006b3b24')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'copy_dispatch_045',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'copy_dispatch_045'
d['batches']['copy_dispatch_045'] = {
    'addresses': list(addresses),
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot 045 (Oct 17 LLM window). Node constructors '
            '006b3094 (signed bounds with >>31 sign words)/006b335c '
            '(plain), plus CRT copy dispatch: 006b3b04 = memcpy thunk '
            '(JMP [008516ac], cdecl 3-arg passthrough) and 006b3b24 = '
            'CPU-feature dispatch init filling the 008516a8 memset / '
            '008516ac memcpy slots.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b3094', 'construct_signed_node (hp6::runtime)', 'src/runtime/PoolList.cpp',
     'copy_dispatch_045: thiscall constructor; 006b20e8 attach, +0x38/'
     '+0x3c cleared, +0x14 record word, bounded pair (+0x28 value, +0x2c '
     'value>>31; +0x20 value, +0x24 value>>31), vtable PTR_FUN_00715ad0, '
     '+0x30, then 006b2199 scheduling.'),
    ('006b335c', 'construct_plain_node (hp6::runtime)', 'src/runtime/PoolList.cpp',
     'copy_dispatch_045: thiscall constructor; attach first, then vtable '
     'PTR_FUN_00715b00 BEFORE +0x14 record word (order differs from the '
     'other constructors), then 006b2199.'),
    ('006b3b04', 'std::memcpy (CRT external)', 'src/runtime/PoolList.cpp',
     'copy_dispatch_045: CRT runtime-selected memcpy thunk: exported asm '
     'PUSH EBP; MOV EBP,ESP; POP EBP; JMP dword ptr [0x008516ac] with no '
     'stack adjustment, so the target keeps cdecl 3-arg (dst, src, cnt); '
     '006b1968 cleans 0xc bytes after. Slot identity proven from the '
     'shipped binary hp6.exe (sha256 41cbb7...): default target 0x006b3b91 '
     'reads [ESP+4/8/0xC], aligns arg1 on 3, 0x20-byte copy loop - scalar '
     'memcpy; SSE2 target 0x006b412e uses MOVEUPS. Consumers use std::memcpy '
     'directly.'),
    ('006b3b24', 'CRT copy dispatch init (CRT external)',
     'src/runtime/PoolList.cpp',
     'copy_dispatch_045: CPU-feature dispatch of the static CRT (from '
     'hp6.exe bytes): tests the feature flag byte at [EBP+0xb] bit 1 '
     '(0x2000000) then bytes 0x40/0x80 at [EBP+0xa], pairing '
     '008516a8 (memset: 0x6b423a SSE2 / 0x6b407a / 0x6b3e30 / 0x6b3c8c '
     'scalar) with 008516ac (memcpy: 0x6b412e / 0x6b3ee4 MMX / 0x6b3d46 / '
     '0x6b3b91 scalar). No bespoke reconstruction; the standard library '
     'performs the dispatch. This resolves the previously deferred thunk '
     'class of 006b3b04.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
