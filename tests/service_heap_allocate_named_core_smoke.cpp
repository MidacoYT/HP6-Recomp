#include <cassert>
#include <cstdint>
#include <cstring>

extern void* service_heap_allocate_core_005dbc50(
    void*, const char*, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t);
extern void* service_heap_allocate_aligned_locked_005de700(
    void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);

void* DAT_00cef710 = nullptr;

namespace {
std::uint8_t heap[0x500]{};
std::uint8_t payload[64]{};
std::uint32_t captured_size, captured_flags, captured_alignment, captured_region;
int registry_calls, locked_calls, aligned_calls, lock_calls, unlock_calls, offset_calls;
bool return_null;
}

const char* service_context_registry_name_005db380(const char* name) {
    assert(std::strcmp(name, "candidate") == 0);
    ++registry_calls;
    return "canonical";
}
void* service_heap_allocate_locked_005dddb0(
    void* arg_heap, std::uint32_t size, std::uint32_t flags) {
    assert(arg_heap == heap);
    ++locked_calls;
    captured_size = size;
    captured_flags = flags;
    return return_null ? nullptr : payload;
}
void* service_heap_allocate_aligned_core_005de780(
    void* arg_heap, std::uint32_t size, std::uint32_t alignment,
    std::uint32_t region, std::uint32_t flags) {
    assert(arg_heap == heap);
    ++aligned_calls;
    captured_size = size;
    captured_alignment = alignment;
    captured_region = region;
    captured_flags = flags;
    return return_null ? nullptr : payload;
}
std::uint32_t service_heap_usable_end_offset_005dede0(void* arg_heap,
                                                         void* arg_payload) {
    assert(arg_heap == heap && arg_payload == payload);
    ++offset_calls;
    return 32;
}
void service_heap_lock_005dd290(void* lock) {
    assert(lock == heap + 0x100);
    ++lock_calls;
}
void service_heap_unlock_005dd2a0(void* lock) {
    assert(lock == heap + 0x100);
    ++unlock_calls;
}

int main() {
    assert(service_heap_allocate_core_005dbc50(
               heap, "candidate", 0, 5, 17, 9) == payload);
    assert(captured_size == 12 && captured_flags == 9);
    assert(locked_calls == 1 && aligned_calls == 0 && registry_calls == 0);
    assert(std::strcmp(*reinterpret_cast<const char**>(payload + 28),
                       "CONTEXTREGISTRYUNAVAILABLE") == 0);

    DAT_00cef710 = heap;
    *reinterpret_cast<void**>(heap + 0x4e4) = heap + 0x100;
    assert(service_heap_allocate_core_005dbc50(
               heap, "candidate", 16, 8, 23, 7) == payload);
    assert(captured_size == 12 && captured_alignment == 16);
    assert(captured_region == 23 && captured_flags == 7);
    assert(locked_calls == 1 && aligned_calls == 1 && registry_calls == 1);
    assert(lock_calls == 1 && unlock_calls == 1 && offset_calls == 2);
    assert(std::strcmp(*reinterpret_cast<const char**>(payload + 28),
                       "canonical") == 0);

    return_null = true;
    assert(service_heap_allocate_core_005dbc50(
               heap, "candidate", 16, 1, 3, 4) == nullptr);
    assert(offset_calls == 2 && lock_calls == 2 && unlock_calls == 2);
}
