#include "runtime/PoolWait.h"

#include "runtime/PoolAccess.h"
#include "runtime/PoolList.h"
#include "runtime/PoolPath.h"
#include "platform/Sync.h"
#include "platform/TimerWheel.h"

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

namespace hp6::runtime {

std::int32_t wait_pool_node(std::uint32_t token) {
    if (token == 0) {
        return -3;
    }
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(token, true));
    if (*reinterpret_cast<std::int32_t*>(frame + 4) == 0) {
        return -3;
    }
    // The caller's argument word both selects the slot and seeds the loop
    // flag ([EBP+0x8] in the exported x86; EBX keeps the initial value as
    // the search key). 006b2147 always clears *source first, so the flag
    // degenerates to the search result.
    const std::uint32_t key = token;
    std::uint32_t flag = token;
    for (;;) {
        platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
        std::uint8_t* found = find_pool_node(
            frame, static_cast<std::int32_t>(key), &flag);
        flag &= found != nullptr ? 1u : 0u;
        platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
        if (flag == 0) {
            break;
        }
        if (platform::thread_owns(0)) {
            platform::pump_timer_slots();
            platform::sleep_alertable(1);
        } else {
#ifdef _WIN32
            const HANDLE event = *reinterpret_cast<HANDLE*>(frame + 0x78);
            WaitForSingleObject(event, INFINITE);
            ResetEvent(event);
#else
            break;
#endif
        }
    }
    return pool_node_status(key);
}

std::int64_t completed_state(std::uint32_t token) {
    return static_cast<std::int64_t>(take_completed_node(token));
}

bool create_and_wait_node(std::uintptr_t record, std::uint8_t mode) {
    const auto token = create_node_from_record(record, mode, 0);
    wait_pool_node(token);
    return take_completed_node(token) != 0;
}

bool create_and_wait_named_node(const char* path, std::uint8_t mode) {
    const auto token = create_named_node_a_from_record(path, mode, 0);
    wait_pool_node(token);
    return take_completed_node(token) != 0;
}

std::int64_t create_and_wait_bounded_path_node(const char* path,
                                               std::uint32_t field_0x20,
                                               std::uint8_t mode) {
    const auto token =
        create_bounded_path_node_from_record(path, field_0x20, mode, 0);
    wait_pool_node(token);
    return static_cast<std::int64_t>(take_completed_node(token));
}

std::int64_t create_and_wait_repeat_node(std::uintptr_t record,
                                         std::int32_t value_1,
                                         std::uint32_t word_0x20,
                                         std::int32_t value_2,
                                         std::uint8_t mode) {
    // Each signed argument is delivered as a CDQ sign-extended pair.
    const auto token = create_repeat_node_from_record(
        record, static_cast<std::uint32_t>(value_1),
        value_1 < 0 ? 0xffffffffu : 0u, word_0x20,
        static_cast<std::uint32_t>(value_2), value_2 < 0 ? 0xffffffffu : 0u,
        mode, 0);
    wait_pool_node(token);
    return static_cast<std::int64_t>(take_completed_node(token));
}

std::int64_t create_and_wait_plain_node(std::uintptr_t record,
                                        std::uint8_t mode) {
    const auto token = create_plain_node_from_record(record, mode, 0);
    wait_pool_node(token);
    return static_cast<std::int64_t>(take_completed_node(token));
}

} // namespace hp6::runtime
