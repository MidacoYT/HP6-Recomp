#include <cstdint>

// PE 0x00892990 initially points to the pool object at 0x00D63EE0.
// Its complete native storage and relocation are still pending.
extern void* DAT_00892990;

void flush_deferred_listeners_00ff87d0(void* header) {
    // Unprotected executable 0x00FF87D0..0x00FF8816. EDI carries header.
    auto* sentinel = static_cast<std::uint8_t*>(header);
    auto* node = *reinterpret_cast<std::uint8_t**>(sentinel);
    if (node == sentinel) return;
    auto* pool = static_cast<std::uint8_t*>(DAT_00892990);
    do {
        auto* next = *reinterpret_cast<std::uint8_t**>(node);
        if (*reinterpret_cast<void**>(node + 8) == nullptr) {
            auto* previous = *reinterpret_cast<std::uint8_t**>(node + 4);
            *reinterpret_cast<void**>(previous) = next;
            *reinterpret_cast<void**>(next + 4) = previous;
            *reinterpret_cast<void**>(node) = *reinterpret_cast<void**>(pool + 8);
            *reinterpret_cast<void**>(pool + 8) = node;
            --*reinterpret_cast<std::uint32_t*>(pool + 0x0c);
        }
        node = next;
    } while (node != sentinel);
}
