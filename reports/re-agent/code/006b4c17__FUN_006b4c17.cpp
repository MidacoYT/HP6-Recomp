#include <cstdint>
#include <cstring>

// Member names and the containing type are unresolved in the supplied evidence.
void __thiscall FUN_006b4c17(void* this_, char* param_1)
{
    auto* bytes = static_cast<unsigned char*>(this_);
    auto* name = reinterpret_cast<char*>(bytes + 0xb4);

    char* cursor = name;
    for (int count = 8; count != 0; --count) {
        cursor[0] = '\0';
        cursor[1] = '\0';
        cursor[2] = '\0';
        cursor[3] = '\0';
        cursor += 4;
    }

    ::strncpy(name, param_1, 0x1f);

    *reinterpret_cast<std::uint32_t*>(bytes + 0x4dc) = 0xffffffffu;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x4e4) = 0;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x4d8) = 4;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x6c) = 4;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x70) = 9;
}