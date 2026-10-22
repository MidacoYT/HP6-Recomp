#include <cstdint>
#include <cstring>

void* DAT_00892990 = nullptr;
void flush_deferred_listeners_00ff87d0(void* header);

namespace {
struct Node {
    void* next;
    void* previous;
    void* listener;
};
}

int main() {
    if (sizeof(void*) != 4) return 0;
    alignas(4) std::uint8_t header[8] = {};
    alignas(4) std::uint8_t pool[0x10] = {};
    Node removed{};
    Node active{};
    removed.next = &active;
    removed.previous = header;
    active.next = header;
    active.previous = &removed;
    active.listener = &active;
    void* head = &removed;
    void* tail = &active;
    std::memcpy(header, &head, 4);
    std::memcpy(header + 4, &tail, 4);
    std::uint32_t count = 2;
    std::memcpy(pool + 0x0c, &count, 4);
    DAT_00892990 = pool;

    flush_deferred_listeners_00ff87d0(header);
    std::memcpy(&count, pool + 0x0c, 4);
    return *reinterpret_cast<void**>(header) == &active &&
           active.previous == header &&
           *reinterpret_cast<void**>(pool + 8) == &removed &&
           removed.next == nullptr && count == 1 ? 0 : 1;
}
