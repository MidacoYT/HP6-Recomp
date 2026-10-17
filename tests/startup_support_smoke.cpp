#include "platform/StartupSupport.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {
std::uint32_t actions[6] = {};
std::uint32_t sizes[6] = {};
std::uint32_t seen_flags[6] = {};
int count = 0;

bool record(std::uint32_t action, std::uint32_t size, void* data,
            std::uint32_t options) {
    if (options != 0 || count >= 6) return false;
    actions[count] = action;
    sizes[count] = size;
    std::memcpy(&seen_flags[count], static_cast<std::byte*>(data) + 4, 4);
    ++count;
    return true;
}

void set_flags(std::byte* data, std::uint32_t value) {
    std::memcpy(data + 4, &value, 4);
}
}

int main() {
    if (hp6::platform::startup_resource_size(8, 16) != 0xb58) return 1;
    hp6::platform::SystemParameterBuffers saved{};
    set_flags(saved.first.data(), 0x0c);
    set_flags(saved.second.data(), 0x01);
    set_flags(saved.third.data(), 0x0e);
    hp6::platform::apply_system_parameters(false, saved, record);
    if (count != 2 || actions[0] != 0x3b || sizes[0] != 8 ||
        seen_flags[0] != 0 || actions[1] != 0x33 || sizes[1] != 24 ||
        seen_flags[1] != 2) return 2;
    hp6::platform::apply_system_parameters(true, saved, record);
    if (count != 5 || actions[2] != 0x3b || seen_flags[2] != 0x0c ||
        actions[3] != 0x35 || seen_flags[3] != 1 ||
        actions[4] != 0x33 || seen_flags[4] != 0x0e) return 3;
    return 0;
}
