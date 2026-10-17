#include "runtime/VolumeRegistry.h"

#include <cstring>

// The service root global is exported as a data symbol without its value;
// reads go through the game address space at binary level.
extern std::int32_t DAT_00892b60;

namespace hp6::runtime {
namespace {

std::uint8_t* pointer(std::uint32_t value) {
    return reinterpret_cast<std::uint8_t*>(value);
}

std::uint32_t word(const std::uint8_t* bytes, std::size_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(bytes + offset);
}

} // namespace

void clear_buffer(std::uintptr_t buffer, std::uint32_t bytes) {
    std::memset(reinterpret_cast<void*>(buffer), 0, bytes);
}

// Volume name getter of the record's object: thiscall with the object in
// ECX and the single stack argument cleaned by the callee (exported x86 of
// 006b25b8: PUSH buf; MOV EAX,[ECX]; CALL [EAX+0x30]).
#ifdef _MSC_VER
using VolumeNameCall = char* (__thiscall*)(void* object, char* buffer);
#else
using VolumeNameCall = char* (*)(void* object, char* buffer);
#endif

std::uint8_t* find_volume_by_name(const char* path) {
    const char* prefix = path;
    const char* colon = std::strchr(path, ':');
    if (colon == nullptr) {
        const auto root = pointer(static_cast<std::uint32_t>(DAT_00892b60));
        if (*path != '/' && *path != '\\') {
            return pointer(word(root, 0x98));
        }
        // Exported x86: ECX = *(root + 0x88), then ECX = *ECX — the game
        // path pointer sits one indirection below the word at root + 0x88.
        prefix = *reinterpret_cast<char* const*>(word(root, 0x88));
        colon = std::strchr(prefix, ':');
    } else {
        colon = std::strchr(path, ':');
    }
    char name[16];
    clear_buffer(reinterpret_cast<std::uintptr_t>(name), 0x10);
    std::strncpy(name, prefix,
                 static_cast<std::size_t>(colon - prefix) + 1);

    const auto root = pointer(static_cast<std::uint32_t>(DAT_00892b60));
    auto* record = pointer(word(root, 0x7c));
    while (record != nullptr) {
        auto* object = pointer(word(record, 0x84));
        const auto vtable =
            static_cast<std::uintptr_t>(word(object, 0));
        const auto slot = reinterpret_cast<VolumeNameCall>(
            *reinterpret_cast<void**>(vtable + 0x30));
        const char* volume_name = slot(object, name);
        if (_stricmp(volume_name, name) == 0) {
            return record;
        }
        record = pointer(word(record, 0));
    }
    return nullptr;
}

} // namespace hp6::runtime
