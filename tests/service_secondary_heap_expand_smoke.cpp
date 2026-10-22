#include <cassert>
#include <cstdint>
#include <cstring>

void* DAT_00cef6b0 = nullptr;
void* DAT_00cef6b8 = nullptr;
std::uint32_t DAT_00cef700 = 0x40;
std::uint32_t DAT_00cef70c = 0;
void* DAT_00fcaea8 = nullptr;

namespace {
std::uint8_t region_storage[0x40]{};
std::uint32_t allocated_size = 0;
std::uint32_t attached_size = 0;
std::uint32_t release_count = 0;
std::uint32_t log_count = 0;
bool allocation_succeeds = true;
bool attach_succeeds = true;
}

void* service_heap_allocate_core_005dbc50(
    void* heap, const char* context, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags) {
    assert(heap == DAT_00cef6b8);
    assert(std::strcmp(context, "GlobalAllocator::Core") == 0);
    assert(alignment == 0x10 && region == 0 && flags == 0);
    allocated_size = size;
    return allocation_succeeds ? region_storage : nullptr;
}

std::uint32_t service_heap_allocate_00ff9850(
    void* heap, void* region, std::uint32_t size, std::uint8_t mode,
    std::uint32_t a, std::uint32_t b, std::uint32_t c) {
    assert(heap == DAT_00cef6b0 && region == region_storage);
    assert(mode == 0 && a == 0 && b == 0 && c == 0);
    attached_size = size;
    return attach_succeeds ? 1 : 0;
}

void service_heap_release_region_005de540(void* heap, void* region) {
    assert(heap == DAT_00cef6b8 && region == region_storage);
    ++release_count;
}

namespace hp6::platform {
void format_diagnostic_message(const char*, ...) { ++log_count; }
}

extern std::uint32_t service_secondary_heap_expand_005dbbb0(std::uint32_t);

int main() {
    int primary = 0;
    int secondary = 0;
    DAT_00cef6b0 = &primary;
    DAT_00cef6b8 = &secondary;
    assert(service_secondary_heap_expand_005dbbb0(17) == 1);
    assert(allocated_size == 32 && attached_size == 32);
    assert(DAT_00cef70c == 32 && release_count == 0 && log_count == 1);

    allocation_succeeds = false;
    assert(service_secondary_heap_expand_005dbbb0(1) == 0);
    assert(DAT_00cef70c == 32 && release_count == 0);

    allocation_succeeds = true;
    attach_succeeds = false;
    assert(service_secondary_heap_expand_005dbbb0(16) == 0);
    assert(release_count == 1 && DAT_00cef70c == 32 && log_count == 1);
}
