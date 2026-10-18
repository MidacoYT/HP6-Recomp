// The types and member names at offsets 0x08 and 0x0C are unresolved.
void __thiscall FUN_006d83ab(void* this_, void* param_1)
{
    std::uint32_t local_c4[48];

    FUN_006d744b(local_c4);
    FUN_006d7b49(this_, reinterpret_cast<std::int32_t>(local_c4));
    FUN_006d7827(param_1, reinterpret_cast<std::int32_t>(local_c4));

    auto* destination = static_cast<std::uint8_t*>(param_1);
    destination[1] &= 0xfb;

    auto* source = static_cast<std::uint8_t*>(this_);
    std::uint32_t byte_count =
        *reinterpret_cast<std::uint32_t*>(source + 0x0c);

    const std::int32_t source_offset =
        *reinterpret_cast<std::int32_t*>(source + 0x08);
    auto* source_words = source_offset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(
              reinterpret_cast<std::uintptr_t>(this_) +
              static_cast<std::uint32_t>(source_offset));

    const std::int32_t destination_offset =
        *reinterpret_cast<std::int32_t*>(destination + 0x08);
    auto* destination_words = destination_offset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(
              reinterpret_cast<std::uintptr_t>(param_1) +
              static_cast<std::uint32_t>(destination_offset));

    for (std::uint32_t count = byte_count >> 2; count != 0; --count) {
        *destination_words = *source_words;
        ++source_words;
        ++destination_words;
    }

    for (byte_count &= 3; byte_count != 0; --byte_count) {
        *reinterpret_cast<std::uint8_t*>(destination_words) =
            *reinterpret_cast<std::uint8_t*>(source_words);
        destination_words = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(destination_words) + 1);
        source_words = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(source_words) + 1);
    }
}
