#include <cassert>
#include <cstdint>

void* DAT_00cef6b0 = nullptr;
void* DAT_00cef6b8 = nullptr;
std::uint32_t seen_size = 0;
std::uint32_t initialized = 0;

namespace hp6::platform {
void** service_holder() {
    ++initialized;
    return nullptr;
}
}

std::uint32_t service_secondary_heap_expand_005dbbb0(std::uint32_t size) {
    seen_size = size;
    return 1;
}

extern std::uint32_t service_primary_heap_growth_005db640(
    void*, std::uint32_t, std::uint32_t, void*);

int main() {
    int primary = 0;
    int secondary = 0;
    DAT_00cef6b0 = &primary;
    DAT_00cef6b8 = &secondary;
    assert(service_primary_heap_growth_005db640(
               &primary, 0x1234, 0x1274, &primary) == 1);
    assert(initialized == 1);
    assert(seen_size == 0x1234);
}
