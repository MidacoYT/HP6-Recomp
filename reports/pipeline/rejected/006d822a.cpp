#include <cstdint>

// The concrete object type and member names are unresolved. Both objects use
// a signed relative data offset at +0x08; the source byte count is at +0x0c.
extern std::uint32_t* __fastcall FUN_006d744b(std::uint32_t*);
extern void __thiscall FUN_006d7a73(void*, std::int32_t);
extern void __thiscall FUN_006d7cd2(void*, std::int32_t);

void __thiscall FUN_006d822a(void* this_, void* param_1)
{
    static_assert(sizeof(void*) == 4);

    std::uint32_t local_c4[0x30];
    FUN_006d744b(local_c4);
    FUN_006d7a73(this_, reinterpret_cast<std::int32_t>(local_c4));
    FUN_006d7cd2(param_1, reinterpret_cast<std::int32_t>(local_c4));

    auto* destination = static_cast<std::uint8_t*>(param_1);
    destination[1] &= 0xfb;

    if (local_c4[20] == 0) { // local_74
        auto* source = static_cast<std::uint8_t*>(this_);
        std::uint32_t count =
            *reinterpret_cast<std::uint32_t*>(source + 0x0c);

        const std::int32_t source_offset =
            *reinterpret_cast<std::int32_t*>(source + 0x08);
        auto* source_data = source_offset == 0 ? nullptr : source + source_offset;

        const std::int32_t destination_offset =
            *reinterpret_cast<std::int32_t*>(destination + 0x08);
        auto* destination_data =
            destination_offset == 0 ? nullptr : destination + destination_offset;

        for (std::uint32_t words = count >> 2; words != 0; --words) {
            *reinterpret_cast<std::uint32_t*>(destination_data) =
                *reinterpret_cast<std::uint32_t*>(source_data);
            source_data += 4;
            destination_data += 4;
        }

        for (count &= 3; count != 0; --count) {
            *destination_data = *source_data;
            ++source_data;
            ++destination_data;
        }
    }
}
