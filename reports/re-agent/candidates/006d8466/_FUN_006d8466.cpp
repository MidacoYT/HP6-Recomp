void __thiscall FUN_006d8466(void* self, void* param_1)
{
    // The supplied evidence does not identify the member names at these offsets.
    auto* source = static_cast<std::uint8_t*>(self);
    auto* destination = static_cast<std::uint8_t*>(param_1);
    std::uint32_t local_c4[48];

    FUN_006d744b(local_c4);
    FUN_006d7b7a(self, reinterpret_cast<std::int32_t>(local_c4));
    destination[1] &= 0xfb;
    FUN_006d78a8(param_1, reinterpret_cast<std::int32_t>(local_c4));

    *reinterpret_cast<std::uint32_t*>(destination + 0x10) =
        *reinterpret_cast<std::uint32_t*>(source + 0x10);
    *reinterpret_cast<std::uint32_t*>(destination + 0x14) =
        *reinterpret_cast<std::uint32_t*>(source + 0x14);

    std::uint32_t count =
        *reinterpret_cast<std::uint32_t*>(source + 0x0c);
    const std::int32_t sourceOffset =
        *reinterpret_cast<std::int32_t*>(source + 0x08);
    const std::int32_t destinationOffset =
        *reinterpret_cast<std::int32_t*>(destination + 0x08);

    auto* sourceWords = sourceOffset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(source + sourceOffset);
    auto* destinationWords = destinationOffset == 0
        ? nullptr
        : reinterpret_cast<std::uint32_t*>(destination + destinationOffset);

    for (std::uint32_t words = count >> 2; words != 0; --words) {
        *destinationWords = *sourceWords;
        ++sourceWords;
        ++destinationWords;
    }

    for (count &= 3; count != 0; --count) {
        *reinterpret_cast<std::uint8_t*>(destinationWords) =
            *reinterpret_cast<std::uint8_t*>(sourceWords);
        sourceWords = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(sourceWords) + 1);
        destinationWords = reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(destinationWords) + 1);
    }
}
