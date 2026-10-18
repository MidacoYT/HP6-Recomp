import json
import subprocess
import sys

# 1. Prepare state entries and batch.
path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
for a in ('006b203a', '006b20e8', '006d9351'):
    if a not in f:
        f[a] = {'batch': 'engine_boot_040',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
for a in ('006d95d0', '006f363c', '006b203a', '006b20e8', '006d9351'):
    f[a]['batch'] = 'engine_boot_040'
d['batches']['engine_boot_040'] = {
    'addresses': ['006d9351', '006d95d0', '006f363c', '006b203a', '006b20e8'],
    'status': 'in_progress',
    'note': 'Manual batch during the Oct 17 06:44 LLM window: CRT import '
            '006f363c (_beginthreadex), trampoline 006d9351 and launcher '
            '006d95d0 unlock the string-pool mid tree; engine bootstrap '
            '006b203a + attach 006b20e8 complete the chain into 006db1a5.',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
PY = r'.\.venv\Scripts\python.exe'
entries = [
    ('006d9351',
     'thread_trampoline_006d9351 (hp6::platform)',
     'src/platform/Sync.cpp',
     'engine_boot_040: CRT thread trampoline named _StartAddress_006d9351 '
     'by Ghidra; writes 1 to ctx[0] then calls ctx[1] with ctx[2]; stdcall, '
     'real start address when invoked through _beginthreadex.'),
    ('006d95d0', 'launch_thread (hp6::platform)', 'src/platform/Sync.cpp',
     'engine_boot_040: ReAgent candidate rejected, integrated against the '
     'exported x86. CREATE_SUSPENDED _beginthreadex with start address '
     '006d9351, priority via 006d94f5, ResumeThread, SleepEx(1,1) wake loop '
     'on the launcher-stack context flag, CloseHandle only when storage is '
     'null. Bool return in AL (MOV AL,1 / XOR AL,AL); CONCAT31 artifact. '
     'param_4 is unread by the binary.'),
    ('006f363c', '_beginthreadex (CRT external)', 'src/platform/Sync.cpp',
     'engine_boot_040: Ghidra-named CRT import thunk; referenced through '
     '<process.h> _beginthreadex inside the launcher reconstruction. No '
     'bespoke reconstruction.'),
    ('006b203a', 'bootstrap_engine (hp6::runtime)', 'src/runtime/PoolAccess.cpp',
     'engine_boot_040: exported x86 confirms the virtual slot +4 call is '
     'thiscall with ECX = object at +0x84 and no stack args; early-exit '
     'branch is a tail jump modeled as a call. Root lock at DAT_00892b60 '
     '+0xc, engine locks +0x28/+0x4c, events +0x44/+0x78, wake word 0xff '
     'at +0x7c, launch of 006b1e32 at +0x68 (param_4 unread).'),
    ('006b20e8', 'attach_engine (hp6::runtime)', 'src/runtime/PoolAccess.cpp',
     'engine_boot_040: thiscall constructor, vtable 007159e8; boots the '
     'engine when its +4 flag is zero; fills +0x10..+0x30 including the '
     'DAT_006b18de table pointer at +0x1c; token via 006b1910 into +0xc.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [PY, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:160])
