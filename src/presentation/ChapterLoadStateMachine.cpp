#include "presentation/ChapterLoadStateMachine.h"
#include "presentation/ChapterFlow.h"

#include <cstdint>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_00cef5b6;
extern std::uint8_t* DAT_00fcadfc; // +4 is a byte flag read/compared here
extern std::uint8_t DAT_00d47e68[0x40 * 0x20]; // zeroed range, stride 0x20, 0x40 entries
extern std::int32_t DAT_00fc7e14;
extern std::int32_t DAT_00cef560;
extern std::int32_t DAT_00d71380;
extern std::int32_t DAT_00d71258;
extern std::int32_t DAT_00d6dee8;
extern std::uint8_t DAT_00cef53f;
extern void* DAT_00cef7b0;
extern void* DAT_00fcade0;
extern std::int32_t DAT_00cef610;

namespace hp6::presentation {
namespace {

// ---- deferred leaf subfunctions -------------------------------------
// None of these are on a path this project's current tests drive the
// machine past state 0 to reach; each is a documented no-op/neutral
// placeholder for a real, not-yet-traced function. Grouped by shape
// rather than given one name per address, since none has an
// independently useful contract yet.
void deferred_void() {}
std::uint32_t deferred_bool_false() { return 0; }
std::uint32_t deferred_uint_zero() { return 0; }
void* deferred_ptr() {
    static std::uint8_t dummy[0x600] = {};
    return dummy;
}

} // namespace

std::uint32_t update_chapter_load_state_machine(std::uint8_t* task) {
    auto* registry = *reinterpret_cast<std::uint8_t**>(task + 4);
    auto& state = *reinterpret_cast<std::int32_t*>(registry + 0x260);
    auto& flags264 = *reinterpret_cast<std::uint32_t*>(registry + 0x264);
    auto set_timer = [&] { *reinterpret_cast<std::int32_t*>(task + 0x10) = 0x8c; };
    auto goto_state = [&](std::int32_t next) {
        state = next;
        set_timer();
    };

    if (state == 0) {
        state = 1;
        set_timer();
    }

    std::uint32_t result = 0;

    switch (state - 1) {
    case 0: { // state 1
        if (DAT_00cef5b6 == 0 && DAT_00fcadfc[4] == 0) {
            deferred_void(); // FUN_0053aca0
        }
        DAT_00cef5b6 = 0;
        task[0xc] = 0;
        deferred_void(); // FUN_00423360
        std::memset(DAT_00d47e68, 0, sizeof(DAT_00d47e68));
        deferred_void(); // thunk_FUN_00fd7070
        if ((flags264 & 1) == 0) {
            deferred_void(); // FUN_004d1300
        }
        const std::int32_t mode = *reinterpret_cast<std::int32_t*>(registry + 0x128);
        if (mode >= 0) {
            if (mode < 2) {
                std::uint32_t ready = deferred_bool_false(); // FUN_0059c9d0
                if (ready != 0) {
                    result = deferred_uint_zero(); // FUN_0059cb20
                }
                goto_state(2);
                return result & 0xffffff00u;
            }
            if (mode == 2) {
                goto_state(0x14);
                return 0;
            }
        }
        break;
    }
    case 1: { // state 2
        deferred_ptr();                                   // FUN_0050b370, slot +0x2c
        deferred_ptr();                                   // FUN_0050b370, slot +0x38
        deferred_void();                                   // FUN_0050b370, field[9]=0
        goto_state(3);
        return result & 0xffffff00u;
    }
    case 2: { // state 3
        std::uint32_t ready_a = deferred_bool_false(); // FUN_0048f240 slot +0x58
        if (ready_a == 0) {
            std::uint32_t ready_b = deferred_bool_false(); // FUN_0050b370 slot +0x58
            if (ready_b == 0) {
                deferred_void(); // FUN_0040c7f0, field[9]=0
                result = deferred_uint_zero(); // FUN_0040c7f0 slot +0x2c
                goto_state(5);
                return result & 0xffffff00u;
            }
        }
        result = deferred_uint_zero(); // FUN_00653df0
        goto_state(4);
        return result & 0xffffff00u;
    }
    case 3: { // state 4
        std::uint32_t done = deferred_bool_false(); // thunk_FUN_00fe4020(0xfc7e28)
        if (done == 0 && DAT_00fc7e14 == 0) {
            goto_state(3);
            return static_cast<std::uint32_t>(DAT_00fc7e14) & 0xffffff00u;
        }
        break;
    }
    case 4: { // state 5
        std::uint32_t done = deferred_bool_false(); // FUN_0040c7f0 slot +0x58
        if (done == 0) {
            goto_state(6);
            return done;
        }
        break;
    }
    case 5: { // state 6
        std::uint32_t done = deferred_bool_false(); // thunk_FUN_00fe4020(0xfc7e28)
        if (done == 0 && DAT_00fc7e14 == 0) {
            deferred_void(); // thunk_FUN_00ff12f0
            goto_state(7);
            return result & 0xffffff00u;
        }
        break;
    }
    case 6: { // state 7
        std::uint32_t done2 = deferred_bool_false(); // thunk_FUN_00fe4020(0xfc7e28)
        if (done2 == 0 && DAT_00fc7e14 == 0) {
            goto_state(8);
            return done2;
        }
        break;
    }
    case 7: { // state 8
        if ((flags264 & 1) == 0) {
            deferred_void(); // FUN_004d1390
        }
        goto_state(9);
        return result & 0xffffff00u;
    }
    case 8: { // state 9
        const std::uint32_t capability = flags264;
        std::uint32_t shifted = capability >> 6;
        if (static_cast<std::int8_t>(capability) < 0) {
            shifted = capability >> 8;
        }
        const std::uint32_t masked = shifted & 0xffffff01u;
        const bool timer_elapsed = *reinterpret_cast<std::int32_t*>(task + 0x10) == 0;
        if (((capability & 1) != 0) || (static_cast<std::uint8_t>(masked) != 0) || timer_elapsed) {
            goto_state(10);
            return shifted & 0xffffff00u;
        }
        break;
    }
    case 9: { // state 10
        if (DAT_00fcadfc[4] == 0) {
            deferred_void(); // FUN_0053aca0
        }
        if ((*reinterpret_cast<std::uint8_t*>(
                 *reinterpret_cast<std::uint8_t**>(
                     static_cast<std::uintptr_t>(DAT_00cef560) + 4) +
                 0x264) &
             2) == 0) {
            deferred_void(); // FUN_005d6b70
            deferred_void(); // FUN_00409220
            deferred_void(); // FUN_0062af60
            deferred_void(); // thunk_FUN_010011e0
            if (DAT_00d71380 != 0) {
                deferred_void(); // thunk_FUN_00ff9700
            }
            if (DAT_00d71258 != 0) {
                deferred_void(); // thunk_FUN_00ff9700
            }
            deferred_void(); // thunk_FUN_0100e510
            result = deferred_uint_zero(); // FUN_004d0400
            goto_state(0xb);
            return result & 0xffffff00u;
        }
        break;
    }
    case 10: { // state 11
        std::uint32_t done = deferred_bool_false(); // FUN_004d0820 -> slot +0x58
        if (done == 0) {
            deferred_void(); // thunk_FUN_00ff9700, conditional
            goto_state(0xc);
            return done;
        }
        break;
    }
    case 11: { // state 12
        flags264 &= 0xfffffffbu;
        deferred_void(); // thunk_FUN_00ff1100
        deferred_void(); // FUN_0062aec0(DAT_00cef7b0)
        goto_state(0xd);
        return result & 0xffffff00u;
    }
    case 12: { // state 13
        bool done;
        if (DAT_00d6dee8 == 0) {
            done = (flags264 & 2) != 0;
        } else {
            done = *reinterpret_cast<std::int32_t*>(
                       static_cast<std::uintptr_t>(DAT_00d6dee8) + 4) == 0;
        }
        if (done) {
            goto_state(0x13);
            flags264 |= 0x20u;
            DAT_00cef53f = 1;
            std::uint32_t pending = deferred_bool_false(); // thunk_FUN_00fe41b0
            if (pending != 0) {
                deferred_void(); // thunk_FUN_00fed350
            }
        }
        if ((flags264 & 4) != 0) {
            deferred_void(); // FUN_0040c7f0, field[9]=1
            goto_state(0xe);
            return result & 0xffffff00u;
        }
        break;
    }
    case 13: { // state 14
        std::uint32_t done = deferred_bool_false(); // FUN_0048f240 slot +0x58
        if (done == 0) {
            goto_state(0xf);
            return done;
        }
        break;
    }
    case 14: { // state 15
        if ((flags264 & 8) == 0) {
            std::uint32_t done = deferred_bool_false(); // FUN_004d0820 slot +0x58
            const std::uint32_t toggle = ((done == 0) ? 8u : 0u) ^ flags264;
            flags264 ^= (toggle & 8u);
            if ((flags264 & 8) == 0) {
                break;
            }
        }
        goto_state(0x10);
        DAT_00cef53f = 1;
        return result & 0xffffff00u;
    }
    case 15: { // state 16
        set_timer();
        if (*reinterpret_cast<std::int32_t*>(
                reinterpret_cast<std::uintptr_t>(DAT_00fcade0) + 4) == 0) {
            deferred_void(); // FUN_0057ba10
            deferred_void(); // thunk_FUN_00fe5650
            std::uint32_t pending = deferred_bool_false(); // thunk_FUN_00fe41b0
            if (pending != 0) {
                deferred_void(); // thunk_FUN_00fed350
            }
            task[8] &= 0xfe;
            goto_state(0x11);
            return result & 0xffffff00u;
        }
        if ((flags264 & 0x20) != 0) {
            std::uint32_t pending = deferred_bool_false(); // thunk_FUN_00fe41b0
            if (pending != 0) {
                result = deferred_uint_zero(); // thunk_FUN_00fed350
                return result & 0xffffff00u;
            }
        }
        break;
    }
    case 16: { // state 17
        std::uint32_t ready = deferred_bool_false(); // FUN_00582110
        if (ready != 0) {
            if (DAT_00fcadfc[4] == 0) {
                deferred_void(); // FUN_0053acd0
            }
            flags264 |= 0x20u;
            std::uint32_t pending = deferred_bool_false(); // thunk_FUN_00fe41b0
            if (pending != 0) {
                deferred_void(); // thunk_FUN_00fed350
            }
            goto_state(0x13);
            return result & 0xffffff00u;
        }
        // NOT YET INTEGRATED: the remaining branch (two more deferred
        // readiness checks gating the same transition) is a documented
        // deferred placeholder.
        break;
    }
    // Note: the exported x86's switch has no case for uVar6==17 (state
    // 18) at all — it jumps straight from 0x10 to 0x12 — so that value
    // falls through to the shared `return result & 0xffffff00` at the
    // bottom, exactly like C++'s implicit default here.
    case 18: { // state 19 (0x12 in the exported x86's own switch) --
               // the one this project cares about.
        goto_state(0);
        registry[0x124] = 0;
        *reinterpret_cast<std::int32_t*>(registry + 0x128) = 0;
        *reinterpret_cast<std::int32_t*>(registry + 300) = 0;
        *reinterpret_cast<std::int32_t*>(registry + 0x130) = 0xff;
        *reinterpret_cast<std::int32_t*>(registry + 0x134) = 0xff;
        registry[0x138] = 0;
        registry[0x139] = 0;
        deferred_void(); // thunk_FUN_00fe55d0
        deferred_void(); // FUN_00431840
        emit_chapter_entry_screen_command(registry);
        if (DAT_00cef610 != 0) {
            // NOT YET INTEGRATED: "STARTLoadChapter: %s" notification.
            return static_cast<std::uint32_t>(DAT_00cef610) & 0xffffff00u;
        }
        if (DAT_00fcadfc[4] == 0) {
            result = deferred_uint_zero(); // FUN_0053acd0
            return result & 0xffffff00u;
        }
        break;
    }
    case 19: { // state 20 (0x13 in the exported x86's own switch)
        if (0 < *reinterpret_cast<std::int32_t*>(registry + 0x268)) {
            deferred_void(); // thunk_FUN_00fe5650
            registry[0x124] = 0;
            *reinterpret_cast<std::int32_t*>(registry + 0x128) = 0;
            *reinterpret_cast<std::int32_t*>(registry + 300) = 0;
            *reinterpret_cast<std::int32_t*>(registry + 0x130) = 0xff;
            *reinterpret_cast<std::int32_t*>(registry + 0x134) = 0xff;
            registry[0x138] = 0;
            registry[0x139] = 0;
            deferred_void(); // thunk_FUN_00fe55d0
            result = deferred_uint_zero(); // FUN_00431840
        }
        break;
    }
    default:
        break;
    }

    return result & 0xffffff00u;
}

} // namespace hp6::presentation
