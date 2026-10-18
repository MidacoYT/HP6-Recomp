import json
import subprocess
import sys
from datetime import datetime, timezone

path = 'reports/pipeline/state.json'
d = json.load(open(path, encoding='utf-8'))
f = d['functions']
addresses = ('006d971d', '006b22c3', '006b3032', '006b318b', '006b2456',
             '006b24b7')
for a in addresses:
    if a not in f:
        f[a] = {'batch': 'pool_wait_043',
                'export': f'.ghidra-exports/{a}.json',
                'integration': 'pending',
                'build': 'not_run',
                'equivalence': 'unverified'}
    f[a]['batch'] = 'pool_wait_043'
d['batches']['pool_wait_043'] = {
    'addresses': list(addresses),
    'status': 'candidates_recorded',
    'integration_review': 'complete',
    'updated_at': datetime.now(timezone.utc).isoformat(),
    'note': 'Manual lot during the Oct 17 LLM window. Timer wheel tick '
            '006d971d (16 slots at 0x899ad8, callback cdecl with '
            '(argument, overdue), due rescheduled from the re-read clock) '
            'plus waiter/constructors 006b22c3/006b3032/006b318b and the '
            'record-fed creators 006b2456/006b24b7 (size word of 006b1d53 '
            'unread but pushed, cdecl).',
}
json.dump(d, open(path, 'w', encoding='utf-8'), indent=2, ensure_ascii=False)
open(path, 'a', encoding='utf-8').write('\n')
print('state prepared')

py = sys.executable
entries = [
    ('006d971d', 'pump_timer_slots (hp6::platform)',
     'src/platform/TimerWheel.cpp',
     'pool_wait_043: stdcall void; when the caller owns the global root it '
     'snapshots DAT_00899c1c = DAT_00899d88 then walks 16 timer slots at '
     '0x00899ad8 (stride 0x14: callback, argument, period, due, busy). Fire '
     'condition: callback set, signed now >= due, busy clear; fires '
     'callback cdecl with (argument, now - due), then due = re-read now + '
     'period and busy cleared around the call.'),
    ('006b22c3', 'wait_pool_node (hp6::runtime)', 'src/runtime/PoolWait.cpp',
     'pool_wait_043: cdecl. -3 when token null or slot record (+4) missing; '
     'loop searches 006b2147 under the +0x28 lock, flag = SearchResult!=0; '
     'between rounds pumps 006d971d + sleep_alertable(1) when owning thread '
     '(thread_owns(0)), otherwise waits/resets the manual-reset event at '
     '+0x78; finally returns 006b2223 classification. Rejected ReAgent '
     'candidate over-modeled the flag.'),
    ('006b3032', 'construct_timer_node (hp6::runtime)', 'src/runtime/PoolList.cpp',
     'pool_wait_043: thiscall constructor; 006b20e8 attach, vtable '
     'PTR_FUN_00715ac0, record word at +0x14, then 006b2199 scheduling.'),
    ('006b318b', 'construct_repeat_node (hp6::runtime)', 'src/runtime/PoolList.cpp',
     'pool_wait_043: thiscall constructor; 006b20e8 attach (mode/w words into '
     '+0x12/+0x18), +0x38/+0x3c cleared, fields +0x14/+0x28/+0x2c/+0x20/'
     '+0x24 in decompiled order, vtable PTR_FUN_00715ae0, +0x30 last, '
     '006b2199 scheduling.'),
    ('006b2456', 'create_node_from_record (hp6::runtime)', 'src/runtime/PoolList.cpp',
     'pool_wait_043: cdecl; detach_first_locked(0x40) - the size word is '
     'pushed by every caller but unread by 006b1d53 (bare RET, caller '
     'cleans, evidenced by 006b2456/2482/24b7/24f2/2527/289e/28d4/2990) - '
     'then 006b3032 with engine = record[0xc]; returns the node +0xc '
     'token, 0 when the pool is empty.'),
    ('006b24b7', 'create_repeat_node_from_record (hp6::runtime)',
     'src/runtime/PoolList.cpp',
     'pool_wait_043: cdecl; same 0x40 slot pull, 006b318b with the '
     'decompiled funnel (p1, p5, p6, p2, p3, p4, p7, p8, record[0xc]); '
     'returns node +0xc token, 0 when empty.'),
]
for addr, sym, src, note in entries:
    res = subprocess.run(
        [py, 'tools/record_integration.py', '--address', addr,
         '--integration', 'integrated', '--symbol', sym, '--source', src,
         '--build', 'passed', '--note', note],
        capture_output=True, text=True, encoding='utf-8', errors='replace')
    print(addr, res.returncode, (res.stdout or res.stderr).strip()[:120])
