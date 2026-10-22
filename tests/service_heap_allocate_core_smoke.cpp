#include <cassert>
#include <cstdint>

extern void* service_heap_allocate_core_005dde30(
    void*, std::uint32_t, std::uint32_t);

namespace {
int direct_calls, commit_calls, release_calls;
std::uint32_t direct_request;
std::uint8_t direct_payload[32]{};
}
void* service_heap_allocate_direct_005df6d0(
    void*, std::uint32_t request) {
    ++direct_calls;
    direct_request = request;
    return direct_payload;
}
void* service_heap_commit_region_005df100(void*, std::uint32_t) {
    ++commit_calls;
    return nullptr;
}
void service_heap_prepare_release_005dec90(void*) { ++release_calls; }
void service_heap_lock_005dd290(void*) {}
void service_heap_unlock_005dd2a0(void*) {}

int main() {
    std::uint8_t heap[0x600]{};
    std::uint8_t storage[0x300]{};
    for (std::uint32_t index = 0; index < 0x80; ++index) {
        auto* sentinel = heap + 0x28 + index * 8;
        *reinterpret_cast<void**>(sentinel + 8) = sentinel;
        *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    }
    *reinterpret_cast<void**>(heap + 0x440) = storage;
    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x101;
    *reinterpret_cast<std::uint32_t*>(heap + 4) = 0x40;
    auto* result = static_cast<std::uint8_t*>(
        service_heap_allocate_core_005dde30(heap, 0x20, 0));
    assert(result == storage + 8);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 4) == 0x29);
    assert(*reinterpret_cast<void**>(heap + 0x440) == storage + 0x28);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 0x2c) == 0xd9);
    assert(direct_calls == 0 && commit_calls == 0 && release_calls == 0);

    assert(service_heap_allocate_core_005dde30(
               heap, 0x2000, 8) == direct_payload);
    assert(direct_calls == 1 && direct_request == 0x2000);

    std::uint8_t small[0x80]{};
    auto* sentinel = heap + 0x28 + 5 * 8;
    *reinterpret_cast<void**>(sentinel + 8) = small;
    *reinterpret_cast<void**>(sentinel + 12) = small;
    *reinterpret_cast<void**>(small + 8) = sentinel;
    *reinterpret_cast<void**>(small + 12) = sentinel;
    *reinterpret_cast<void**>(heap + 0x468) = small + 0x70;
    assert(service_heap_allocate_core_005dde30(
               heap, 0x20, 0) == small + 8);
    assert(*reinterpret_cast<void**>(sentinel + 8) == sentinel);
    assert(*reinterpret_cast<void**>(sentinel + 12) == sentinel);
    assert((*reinterpret_cast<std::uint32_t*>(small + 0x2c) & 1u) != 0);

    *reinterpret_cast<void**>(heap + 0x14) = storage;
    *reinterpret_cast<void**>(storage + 0x0c) = nullptr;
    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x2d;
    assert(service_heap_allocate_core_005dde30(
               heap, 0x20, 0) == storage + 8);
    assert(*reinterpret_cast<void**>(heap + 0x14) == nullptr);
    assert((*reinterpret_cast<std::uint32_t*>(storage + 4) & 4u) == 0);

    *reinterpret_cast<void**>(heap + 0x440) = storage;
    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x101;
    assert(service_heap_allocate_core_005dde30(
               heap, 0x20, 1) == storage + 0xe0);
    assert(*reinterpret_cast<void**>(heap + 0x440) == storage);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 4) == 0xd9);

    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x11;
    heap[0x46d] = 1;
    assert(service_heap_allocate_core_005dde30(
               heap, 0x400, 0) == nullptr);
    assert(commit_calls == 1);
}
