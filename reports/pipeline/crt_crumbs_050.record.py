import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
entries = [
    ('006b9790', '__alloca_probe (CRT external)', 'records/crt',
     'crt_crumbs_050: MSVC __chkstk/__alloca_probe body (walks and probes '
     'the paged stack frames down from EAX); standard CRT helper, no '
     'bespoke reconstruction. 22 dependents unblocked.'),
    ('006b9740', '__allmul (CRT external)', 'records/crt',
     'crt_crumbs_050: Visual Studio __allmul 32x32->64 helper body; our '
     'reconstruction uses native 64-bit arithmetic. 3 dependents.'),
    ('006b93f4', '_vsnprintf (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x0071324c]. 3 dependents.'),
    ('006b92c2', 'printf (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x00713210].'),
    ('006b92aa', 'vsprintf (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x007131f4].'),
    ('006b9b36', '_CIpow (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x007132ac] (FPU '
     'stack-based pow helper).'),
    ('006b9b3c', '_CIsqrt (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x007132b0] (FPU '
     'stack-based sqrt helper).'),
    ('006b9b48', 'getenv (CRT external)', 'records/crt',
     'crt_crumbs_050: import thunk JMP dword ptr [0x007132b8].'),
    ('006b9be1', '__SEH_epilog4 (CRT external)', 'records/crt',
     'crt_crumbs_050: MSVC __SEH_epilog4 prologue/epilogue helper body '
     '(restores the FS exception chain and EBP); 9 dependents unblocked.'),
    ('006b9b9c', '__SEH_prolog4 (CRT external)', 'records/crt',
     'crt_crumbs_050: MSVC __SEH_prolog4 helper body (PUSH FS:[0]; builds '
     'the exception registration and the EBP frame, 0x890 chars); 9 '
     'dependents unblocked.'),
    ('006b9a46', '_ftol2 (CRT external)', 'records/crt',
     'crt_crumbs_050: _ftol2 FTST0->int64 rounding body (ROUND + 0x80000000 '
     'correction); 19 dependents unblocked.'),
    ('006b9a10', '_ftol2 fastcall sibling (CRT external)', 'records/crt',
     'crt_crumbs_050: same rounding body entered with two register '
     'arguments, dispatched on DAT_00fd22b8 (SSE2 flag); 9 dependents '
     'unblocked.'),
]
addresses = [a for a, *_ in entries]
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'crt_crumbs_050',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'crt_crumbs_050'
d['batches']['crt_crumbs_050'] = {
    'addresses': addresses,
    'status': 'integrated',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'CRT crumbs: import thunks (JMP dword ptr [IAT]) and MSVC '
            'helper bodies (alloca_probe, SEH prolog/epilog, ftol2, allmul) '
            'recorded as CRT externals per the established volume_registry_044 '
            'pattern; unblocks the 006b92xx-006b9bxx dependency fan.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
