#include "presentation/NativeScreenRecordMethods.h"

#include <array>
#include <cstdint>

void* DAT_00fcadf0 = nullptr;

int main() {
    using hp6::presentation::NativeScreenRecord;
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x20> handler{};
    alignas(void*) std::array<std::uint8_t, 0x10> list_node{};
    alignas(void*) std::array<std::uint8_t, 0x10> task{};
    DAT_00fcadf0 = handler.data();
    *reinterpret_cast<void**>(handler.data() + 0xc) = list_node.data();
    *reinterpret_cast<void**>(list_node.data() + 8) = task.data();

    NativeScreenRecord record{nullptr, "ScreenLanguageSelect", task.data()};
    if (!record.has_task(task.data()) || record.has_task(nullptr)) return 1;
    record.cancel_pending_task();
    if (record.pending_task != nullptr || task[4] != 1) return 2;
    record.pending_task = task.data();
    task[4] = 0;
    *reinterpret_cast<void**>(handler.data() + 0xc) = nullptr;
    record.cancel_pending_task();
    if (record.pending_task != nullptr || task[4] != 0) return 3;
    record.pending_task = task.data();
    record.clear_pending_task();
    return record.pending_task == nullptr ? 0 : 4;
}
