#include <cstdint>
#include <cstdio>

extern std::uint32_t* __fastcall FUN_006d744b(std::uint32_t*);
extern void __thiscall FUN_006d7baf(void*, std::uint32_t*);
extern void __thiscall FUN_006d793d(void*, std::uint32_t*);

void __thiscall FUN_006d852e(void* this_, void* param_1)
{
    std::uint32_t local_c8[48];

    FUN_006d744b(local_c8);
    FUN_006d7baf(this_, local_c8);
    FUN_006d793d(param_1, local_c8);

    auto* source = static_cast<std::uint8_t*>(this_);
    auto* destination = static_cast<std::uint8_t*>(param_1);
    destination[1] &= 0xfb;

    // The fields at +0x08 and +0x0c have no verified member names.
    std::uint32_t count = *reinterpret_cast<std::uint32_t*>(source + 0x0c);
    const auto source_offset = *reinterpret_cast<std::int32_t*>(source + 0x08);
    const auto destination_offset =
        *reinterpret_cast<std::int32_t*>(destination + 0x08);

    auto* src = source_offset == 0 ? nullptr : source + source_offset;
    auto* dst = destination_offset == 0
        ? nullptr : destination + destination_offset;

    for (std::uint32_t words = count >> 2; words != 0; --words) {
        *reinterpret_cast<std::uint32_t*>(dst) =
            *reinterpret_cast<std::uint32_t*>(src);
        src += 4;
        dst += 4;
    }

    for (count &= 3; count != 0; --count) {
        *dst = *src;
        ++src;
        ++dst;
    }

    if (0x14 < *reinterpret_cast<std::int32_t*>(source + 0x0c) + 0x10) {
        char* text = destination_offset == 0
            ? nullptr
            : reinterpret_cast<char*>(destination + destination_offset);
        std::sprintf(
            text, "EAGL%d",
            *reinterpret_cast<std::int32_t*>(destination + 0x0c));
    }
}