// The structure type and member names are unresolved. Offsets are retained.
void __thiscall FUN_006d8619(void* this_, void* param_1)
{
    std::uint32_t local_c4[48];

    FUN_006d744b(local_c4);
    FUN_006d7bd2(this_, reinterpret_cast<std::int32_t>(local_c4));
    FUN_006d79ce(param_1, reinterpret_cast<std::int32_t>(local_c4));

    auto* destination = static_cast<std::uint8_t*>(param_1);
    destination[1] &= 0xfb;

    auto* source = static_cast<std::uint8_t*>(this_);
    std::uint32_t count = *reinterpret_cast<std::uint32_t*>(source + 0x0c);

    const std::int32_t source_offset =
        *reinterpret_cast<std::int32_t*>(source + 0x08);
    auto* source_words = source_offset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(source + source_offset);

    const std::int32_t destination_offset =
        *reinterpret_cast<std::int32_t*>(destination + 0x08);
    auto* destination_words = destination_offset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(destination + destination_offset);

    for (std::uint32_t words = count >> 2; words != 0; --words) {
        *destination_words = *source_words;
        ++source_words;
        ++destination_words;
    }

    for (count &= 3; count != 0; --count) {
        *reinterpret_cast<std::uint8_t*>(destination_words) =
            *reinterpret_cast<std::uint8_t*>(source_words);
        source_words = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(source_words) + 1);
        destination_words = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(destination_words) + 1);
    }
}