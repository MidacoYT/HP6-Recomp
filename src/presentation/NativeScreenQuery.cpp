#include <cstdint>
#include <cstring>

// 0x0048ACE0 and 0x0053CC00 in the SHA-verified unprotected x86 image.
// The original image initializes this pointer to zero.
std::uint8_t* DAT_00fcade8 = nullptr;

namespace {
#ifdef _MSC_VER
using NameMethod = const char* (__thiscall*)(void*);
#else
using NameMethod = const char* (*)(void*);
#endif

const char* screen_name(std::uint8_t* object) {
    auto* vtable = *reinterpret_cast<std::uint8_t**>(object);
    auto* method = *reinterpret_cast<NameMethod*>(vtable + 0x34);
    return method(object);
}
} // namespace

void* __stdcall screen_find_by_name_0053cc00(std::uint8_t* manager,
                                               const char* name) {
    auto* sentinel = manager + 0x4518;
    for (auto* node = *reinterpret_cast<std::uint8_t**>(sentinel);
         node != sentinel;
         node = *reinterpret_cast<std::uint8_t**>(node)) {
        auto* screen = *reinterpret_cast<std::uint8_t**>(node + 8);
        if (std::strcmp(screen_name(screen), name) == 0) return screen;
    }
    return nullptr;
}

std::int32_t command_screen_active_0048ace0(const char* name) {
    auto* screen = screen_find_by_name_0053cc00(DAT_00fcade8, name);
    if (screen == nullptr) return 0;
    auto* sentinel = DAT_00fcade8 + 0x4524;
    for (auto* node = *reinterpret_cast<std::uint8_t**>(sentinel);
         node != sentinel;
         node = *reinterpret_cast<std::uint8_t**>(node)) {
        if (*reinterpret_cast<void**>(node + 8) == screen) return 1;
    }
    return 0;
}
