#include "runtime/PoolAccess.h"
#include "platform/Sync.h"

#include <bit>

#ifdef _WIN32
#include <windows.h>
#endif

// Engine bootstrap externs: the engine worker body (not part of this batch),
// the stack-size word read from game space, and the data symbols written
// into the attach receiver.
extern std::int32_t engine_worker_006b1e32(std::int32_t engine);
extern std::int32_t DAT_0085139c;
extern void* PTR_FUN_007159e8;
extern void* DAT_006b18de;

// The service root global is exported as a data symbol without its value;
// reads go through the game address space at binary level.
extern std::int32_t DAT_00892b60;

namespace hp6::runtime {

std::uint32_t allocate_slot_token(std::uintptr_t slot_address) {
    auto* root = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(DAT_00892b60)));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(root + 0x54));
    root = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(DAT_00892b60)));
    auto& counter = *reinterpret_cast<std::uint32_t*>(root + 0x50);
    const std::uint32_t previous = counter;
    counter = previous + 1u;
    if (counter == 0x10000u) counter = 1;
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(root + 0x54));

    root = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(DAT_00892b60)));
    const auto base = *reinterpret_cast<const std::uint32_t*>(root + 0x98);
    const auto offset = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(slot_address) - base);
    return static_cast<std::uint32_t>(offset / 0x88) | (previous << 8);
}

std::uintptr_t pool_slot_address(std::uint32_t index, bool reset) {
    const auto root = static_cast<std::uintptr_t>(DAT_00892b60);
    const auto base = *reinterpret_cast<const std::int32_t*>(root + 0x98);
    const auto result = static_cast<std::uintptr_t>(
        static_cast<std::int32_t>((index & 0xff)) * 0x88 + base);
    const auto* owner_word = reinterpret_cast<const std::int32_t*>(result + 0x80);
    if (*owner_word != 0 && reset) {
        return static_cast<std::uintptr_t>(base);
    }
    return result;
}

void read_virtual_pair(const std::uint8_t* frame, std::uint64_t* out_value,
                       std::uint32_t* out_words) {
    const auto base = reinterpret_cast<std::uintptr_t>(frame);
    // Vtable slot at +0x24 (nine 4-byte entries in).
    const auto object =
        *reinterpret_cast<std::uintptr_t* const*>(base + 0x10);
    const auto* vtable =
        reinterpret_cast<const std::uintptr_t*>(*object);
#ifdef _MSC_VER
    using pair_fn = std::uint64_t (__thiscall*)(void*, std::uint32_t);
#else
    using pair_fn = std::uint64_t (*)(void*, std::uint32_t);
#endif
    const auto fn = reinterpret_cast<pair_fn>(vtable[0x24 / 4]);
    *out_value = fn(object, *reinterpret_cast<const std::uint32_t*>(base + 8));
    *out_words = *reinterpret_cast<const std::uint32_t*>(base + 0x18);
    out_words[1] = *reinterpret_cast<const std::uint32_t*>(base + 0x1c);
}

#ifdef _MSC_VER
// Exported x86 of 006b203a: MOV ECX,[record+0x84]; MOV EAX,[ECX];
// CALL/JMP [EAX+4] — thiscall with the object in ECX and no stack arguments.
// The early-exit branch is a tail jump (POP EDI/ESI; JMP [EAX+4]); modeled
// as a plain call, one stack frame deeper, with identical side effects.
using EngineSlotCall = void (__thiscall*)(void*);
#else
using EngineSlotCall = void (*)(void*);
#endif

void run_engine_slot_4(std::uintptr_t record) {
    auto* object = *reinterpret_cast<void**>(record + 0x84);
    const auto vtable =
        reinterpret_cast<std::uintptr_t>(*reinterpret_cast<void**>(object));
    reinterpret_cast<EngineSlotCall>(
        *reinterpret_cast<void**>(vtable + 4))(object);
}

// 006b203a. Engine-thread bootstrap over the service-pool root.
void bootstrap_engine(std::uintptr_t record) {
    auto* base = reinterpret_cast<std::uint8_t*>(record);
    if (*reinterpret_cast<std::int32_t*>(base + 0x80) != 0) {
        *reinterpret_cast<std::int32_t*>(base + 4) = 1;
        run_engine_slot_4(record);
        return;
    }
    std::int32_t root = DAT_00892b60;
    platform::enter_lock(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(root)) + 0xc);
    platform::initialize_lock(record + 0x28);
    *reinterpret_cast<std::int32_t*>(base + 0xc) = 0;
    platform::create_event(record + 0x44);
#ifdef _WIN32
    *reinterpret_cast<HANDLE*>(base + 0x78) =
        CreateEventA(nullptr, TRUE, FALSE, nullptr);
#endif
    platform::initialize_lock(record + 0x4c);
    *reinterpret_cast<std::uint32_t*>(base + 0x7c) = 0xff;
    run_engine_slot_4(record);
    platform::launch_thread(record + 0x68,
                            reinterpret_cast<void (*)(void*)>(
                                engine_worker_006b1e32),
                            reinterpret_cast<void*>(record),
                            *reinterpret_cast<std::uint32_t*>(base + 0x74),
                            static_cast<std::uint32_t>(DAT_0085139c), 1);
#ifdef _WIN32
    WaitForSingleObject(*reinterpret_cast<HANDLE*>(base + 0x78), INFINITE);
    ResetEvent(*reinterpret_cast<HANDLE*>(base + 0x78));
#endif
    platform::leave_lock(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(DAT_00892b60)) +
        0xc);
}

// 006b20e8. Engine attach constructor (thiscall in the binary; modeled as a
// plain function with the receiver as first argument).
std::uint8_t* attach_engine(std::uint8_t* receiver, std::uint8_t mode,
                            std::uint32_t words, std::uintptr_t engine) {
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_007159e8;
    if (*reinterpret_cast<std::int32_t*>(engine + 4) == 0) {
        bootstrap_engine(engine);
    }
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = 0xffffffff;
    receiver[0x12] = mode;
    receiver[0x10] = 0;
    receiver[0x11] = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x18) = words;
    *reinterpret_cast<void**>(receiver + 0x1c) = &DAT_006b18de;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x20) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x24) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x28) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x2c) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x30) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0xc) =
        allocate_slot_token(engine);
    return receiver;
}

// 006b2553 (cdecl). Runs a callback while the timer wheel lock is held and
// a reserve word is visibly held at frame + 0x7c. The carrying frame is the
// record itself, unless it is slot-flagged (+0x80 non-zero), in which case
// the wheel base (root + 0x98) carries the lock and event. The record is
// bootstrapped first when its +4 flag is clear.
std::uint32_t run_reserved_callback(
    std::uint32_t (*callback)(std::uint32_t, std::int32_t),
    std::uintptr_t record, std::uint32_t reserve_value,
    std::int32_t callback_arg) {
    auto* frame = reinterpret_cast<std::uint8_t*>(record);
    if (*reinterpret_cast<std::int32_t*>(frame + 4) == 0) {
        bootstrap_engine(record);
    }
    if (*reinterpret_cast<std::int32_t*>(frame + 0x80) != 0) {
        const auto root = static_cast<std::uint32_t>(DAT_00892b60);
        frame = reinterpret_cast<std::uint8_t*>(
            *reinterpret_cast<const std::uint32_t*>(root + 0x98));
    }
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x4c));
    const auto held = *reinterpret_cast<std::uint32_t*>(frame + 0x7c);
    *reinterpret_cast<std::uint32_t*>(frame + 0x7c) = reserve_value;
    const auto result = callback(reserve_value, callback_arg);
    *reinterpret_cast<std::uint32_t*>(frame + 0x7c) = held;
    platform::signal_event(reinterpret_cast<std::uintptr_t>(frame + 0x44));
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x4c));
    return result;
}

} // namespace hp6::runtime
