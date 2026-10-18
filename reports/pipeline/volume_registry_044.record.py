import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006b3c8c', '006b92bc', '006b9392', '006b93a4', '006b3b0e',
             '006b25b8')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'volume_registry_044',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'volume_registry_044'
d['batches']['volume_registry_044'] = {
    'addresses': list(addresses),
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot during the Oct 17 LLM window. CRT records '
            '(006b3c8c static memset body, 006b92bc strncpy import, '
            '006b9392 strncmp import, 006b93a4 _stricmp import) plus the '
            'real clear_buffer 006b3b0e and volume lookup 006b25b8 with '
            'the third confirmed virtual-call convention (thiscall ECX '
            'this + one callee-cleaned stack argument).',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006b3c8c', 'std::memset (CRT external)', 'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: static CRT memset body compiled into the binary '
     '(Ghidra decompiled the real implementation: unaligned head, 32-byte '
     'fill loop, tail). No bespoke reconstruction; consumers use '
     'std::memset directly.'),
    ('006b92bc', 'std::strncpy (CRT external)', 'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: CRT strncpy import thunk; the jumptable warning '
     'is thunk recursion modeling noise. Referenced through <cstring> in '
     'the volume prefix extraction.'),
    ('006b9392', 'std::strncmp (CRT external)', 'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: CRT strncmp import thunk; same thunk class as '
     '006b92bc. Kept for the volume lookup dependency graph.'),
    ('006b93a4', '::_stricmp (CRT external)', 'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: CRT _stricmp import thunk; called cdecl with '
     '(slot name, local prefix) in the volume scan.'),
    ('006b3b0e', 'clear_buffer (hp6::runtime)', 'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: cdecl with two stack args cleaned by the '
     'caller (ADD ESP,0x1c in 006b25b8 covers memset through strnicmp '
     'pushes). Zeroes bytes at buffer via the memset slot '
     'PTR_FUN_008516a8; implemented as a std::memset wrapper.'),
    ('006b25b8', 'find_volume_by_name (hp6::runtime)',
     'src/runtime/VolumeRegistry.cpp',
     'volume_registry_044: exported x86 confirms the +0x30 virtual slot '
     'call is thiscall with ECX = object at record + 0x84 AND one stack '
     'argument (the local name buffer) cleaned by the callee - third '
     'convention beyond the stdcall-slot and ECX-only-no-arg variants. '
     'The slot must NOT overwrite the buffer (otherwise the _stricmp '
     'would compare the record name against itself). Prefix extraction: '
     'two dereferences only from root + 0x88 (ECX=[root+0x88]; ECX=[ECX]) '
     '- a triple deref crashes, disproving the Ghidra three-level read; '
     'zeroed 16-byte local then strncpy of len(colon)+1 bytes relies on '
     'the zeroed tail for the terminator.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
