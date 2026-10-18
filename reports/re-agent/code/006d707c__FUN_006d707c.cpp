std::uint32_t __thiscall FUN_006d707c(void* self, std::uint32_t* param_1)
{
    const std::uint8_t wanted = FUN_006d6c09(param_1);
    auto* current = static_cast<std::uint32_t*>(self);
    std::uint32_t* previous = nullptr;

    for (;;) {
        if (current == nullptr) {
            FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(param_1));
            param_1[1] = 0; // Unresolved field at offset 0x04.
            return 1;
        }

        const std::uint8_t found = FUN_006d6c09(current);
        const std::uint32_t offset = current[1]; // Unresolved field at offset 0x04.

        if (wanted == found) {
            const auto next = offset == 0
                ? std::int32_t{0}
                : static_cast<std::int32_t>(
                      reinterpret_cast<std::uintptr_t>(current) + offset);

            FUN_006d6bf1(param_1, next);
            FUN_006d6bf1(current, reinterpret_cast<std::int32_t>(param_1));
            return 1;
        }

        previous = current;
        current = offset == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(
                  reinterpret_cast<std::uintptr_t>(current) + offset);
    }
}