#include "platform/ServiceHolderDestructor.h"

#include <cstdint>
#include <cstring>
#include <array>
#include <string>

void* DAT_00cef6b0 = nullptr;
void* DAT_00cef6b4 = nullptr;
extern char DAT_00cef6c0[0x40];
extern void* DAT_00cef710;

namespace {
struct Option {
    std::uint32_t code;
    std::uint32_t value;
    const Option* next;
};
int init_calls = 0;
int allocate_calls = 0;
void* last_heap = nullptr;
std::string last_name;
std::uint32_t last_alignment = 0;
std::uint32_t last_size = 0;
std::uint32_t last_region = 0;
std::uint32_t last_flags = 0;
int primary;
int secondary_heap;
int result;
std::array<std::uint8_t, 64> context_payload{};
std::uint32_t locked_size = 0;
std::uint32_t locked_flags = 0;
} // namespace

void service_holder_heap_initialize_005dba40(void*) {
    ++init_calls;
    DAT_00cef6b0 = &primary;
    DAT_00cef6b4 = &secondary_heap;
}

void* service_heap_allocate_core_005dbc50(
    void* heap, const char* context, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags) {
    ++allocate_calls;
    last_heap = heap;
    last_name = context;
    last_alignment = alignment;
    last_size = size;
    last_region = region;
    last_flags = flags;
    return &result;
}

const char* service_context_registry_name_005db380(const char*) {
    return "registered-context";
}

void* service_heap_allocate_locked_005dddb0(
    void* heap, std::uint32_t size, std::uint32_t flags) {
    last_heap = heap;
    locked_size = size;
    locked_flags = flags;
    return context_payload.data();
}

std::uint32_t service_heap_usable_end_offset_005dede0(void*, void*) {
    return 20;
}

int main() {
    static_assert(sizeof(void*) == 4);
    hp6::platform::ServiceHolderObject holder{};
    auto* service = reinterpret_cast<hp6::platform::ServiceHolderSecondary*>(
        reinterpret_cast<std::uint8_t*>(&holder) + 4);
    const Option end{0, 0, nullptr};
    if (service->allocate_from_options(17, &end) != &result ||
        init_calls != 1 || allocate_calls != 1 || last_heap != &primary ||
        last_name != "Unnamed::" || last_alignment != 16 ||
        last_size != 17 || last_region != 0 || last_flags != 0) return 1;

    const char name[] = "test allocation";
    const Option switch_heap{0x10000, 0, nullptr};
    const Option flags{4, 7, &switch_heap};
    const Option region{3, 9, &flags};
    const Option alignment{2, 32, &region};
    const Option named{1, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(name)), &alignment};
    if (service->allocate_from_options(24, &named) != &result ||
        last_heap != &secondary_heap || last_name != name ||
        last_alignment != 32 || last_size != 24 ||
        last_region != 9 || last_flags != 7) return 2;

    const Option late_name{1, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(name)), nullptr};
    const Option early_switch{0x10000, 0, &late_name};
    service->allocate_from_options(1, &early_switch);
    if (last_heap != &primary) return 3;
    std::strcpy(DAT_00cef6c0, "RenderFramework");
    service->allocate_from_options(1, &named);
    if (last_name != "RenderFramework::test allocation") return 4;
    if (holder.allocate_named(43, name, 5, 64, 11) != &result ||
        last_heap != &primary || last_name != "RenderFramework::test allocation" ||
        last_size != 43 || last_alignment != 64 ||
        last_region != 11 || last_flags != 5) return 5;
    if (holder.allocate_context(17, name, 3) != context_payload.data() ||
        last_heap != &primary || locked_size != 24 || locked_flags != 3 ||
        std::strcmp(*reinterpret_cast<const char**>(context_payload.data() + 16),
                    "CONTEXTREGISTRYUNAVAILABLE") != 0) return 6;
    DAT_00cef710 = &result;
    if (holder.allocate_context(8, nullptr, 2) != context_payload.data() ||
        locked_size != 12 ||
        std::strcmp(*reinterpret_cast<const char**>(context_payload.data() + 16),
                    "registered-context") != 0) return 7;
    return 0;
}
