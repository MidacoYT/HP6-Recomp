void __thiscall FUN_006d82ec(void* this_, void* param_1)
{
    struct LocalState {
        std::uint32_t words[24];
        std::int32_t field_60;
    } local;

    FUN_006d744b(local.words);
    FUN_006d7b14(this_, reinterpret_cast<std::intptr_t>(local.words));
    FUN_006d778a(param_1, reinterpret_cast<std::intptr_t>(local.words));

    auto* destination = static_cast<std::uint8_t*>(param_1);
    destination[1] &= 0xfb;

    if (local.field_60 == 0) {
        auto* source = static_cast<std::uint8_t*>(this_);
        std::uint32_t count = *reinterpret_cast<std::uint32_t*>(source + 0x0c);

        const std::int32_t sourceOffset =
            *reinterpret_cast<std::int32_t*>(source + 0x08);
        auto* sourceWords = sourceOffset == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(source + sourceOffset);

        const std::int32_t destinationOffset =
            *reinterpret_cast<std::int32_t*>(destination + 0x08);
        auto* destinationWords = destinationOffset == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(destination + destinationOffset);

        for (std::uint32_t words = count >> 2; words != 0; --words) {
            *destinationWords++ = *sourceWords++;
        }

        auto* sourceBytes = reinterpret_cast<std::uint8_t*>(sourceWords);
        auto* destinationBytes = reinterpret_cast<std::uint8_t*>(destinationWords);
        for (count &= 3; count != 0; --count) {
            *destinationBytes++ = *sourceBytes++;
        }
    }
}