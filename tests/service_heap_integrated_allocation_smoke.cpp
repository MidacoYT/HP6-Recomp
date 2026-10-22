#include <cassert>
#include <cstdint>

extern void* service_heap_allocate_aligned_core_005de780(
    void*, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t);

namespace { int frees; }
void* service_heap_allocate_direct_005df6d0(void*, std::uint32_t) {
    assert(false);
    return nullptr;
}
void* service_heap_commit_region_005df100(void*, std::uint32_t) {
    assert(false);
    return nullptr;
}
void service_heap_prepare_release_005dec90(void*) { assert(false); }
void service_heap_lock_005dd290(void*) { assert(false); }
void service_heap_unlock_005dd2a0(void*) { assert(false); }
void service_heap_free_payload_005de580(void*, void*) { ++frees; }

int main() {
    std::uint8_t heap[0x600]{};
    alignas(64) std::uint8_t storage[0x1200]{};
    for (std::uint32_t index = 0; index < 0x80; ++index) {
        auto* sentinel = heap + 0x28 + index * 8;
        *reinterpret_cast<void**>(sentinel + 8) = sentinel;
        *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    }
    *reinterpret_cast<void**>(heap + 0x440) = storage;
    *reinterpret_cast<void**>(heap + 0x468) = storage + 0x800;
    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x1001;
    *reinterpret_cast<std::uint32_t*>(storage + 0x1004) = 9;
    *reinterpret_cast<std::uint32_t*>(storage + 0x100c) = 1;

    auto* payload = static_cast<std::uint8_t*>(
        service_heap_allocate_aligned_core_005de780(
            heap, 0x60, 64, 8, 0));
    assert(payload != nullptr);
    assert(((reinterpret_cast<std::uintptr_t>(payload) + 8) & 63u) == 0);
    assert(*reinterpret_cast<void**>(heap + 0x440) > storage);
    assert(frees >= 1);
}
