#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

extern void* PTR_FUN_0083e55c[9];
std::int32_t DAT_00998580 = 0;

namespace hp6::platform {
void* ServiceHolderObject::allocate_named(std::uint32_t, const char*,
                                          std::uint32_t, std::uint32_t,
                                          std::uint32_t) { return nullptr; }
void* ServiceHolderObject::allocate_context(std::uint32_t, const char*,
                                            std::uint32_t) { return nullptr; }
void* ServiceHolderSecondary::allocate_from_options(std::uint32_t,
                                                    const void*) { return nullptr; }
void release_through_service(void*) {}
} // namespace hp6::platform

void cleanup_service_holder_005db9a0(void*) {}
void service_handle_free_005db8d0(void*, std::uint32_t) {}

int main() {
    static_assert(sizeof(void*) == 4);
    for (void* slot : PTR_FUN_0083e55c) {
        if (slot == nullptr) return 1;
    }
    if (PTR_FUN_0083e55c[6] != PTR_FUN_0083e55c[7]) return 2;
    using StateFunction = std::uint32_t (*)();
    if (reinterpret_cast<StateFunction>(PTR_FUN_0083e55c[6])() != 1) return 3;
    if (PTR_FUN_0083e55c[0] == PTR_FUN_0083e55c[8]) return 4;
    hp6::platform::ServiceHolderObject holder{};
    using SecondaryDestroy = void* (__thiscall*)(void*, std::uint32_t);
    auto* secondary = reinterpret_cast<std::uint8_t*>(&holder) + 4;
    if (reinterpret_cast<SecondaryDestroy>(PTR_FUN_0083e55c[8])(
            secondary, 0) != &holder) return 5;
    return 0;
}
