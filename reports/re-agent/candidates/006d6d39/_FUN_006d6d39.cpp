std::uint32_t __thiscall FUN_006d6d39(
    void* this_,
    std::uint32_t param_1,
    std::uint32_t* param_2)
{
    // The structures at this_ and param_2 are unresolved. Their fields
    // at +0x04 and +0x0c are therefore accessed by offset.
    auto field = [](void* object, std::uint32_t offset) -> std::uint32_t& {
        return *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(object) + offset));
    };

    const std::uint32_t requested_type = FUN_006d6c09(param_2);
    auto cursor = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(&param_1));
    std::uint32_t previous = 0;
    void* selected = nullptr;
    void* current = this_;

    for (;;) {
        if (current == nullptr) {
            *reinterpret_cast<std::uint32_t*>(previous + 4) = 0;
            return 1;
        }

        const std::uint32_t type = FUN_006d6c09(
            reinterpret_cast<std::uint32_t*>(current));

        void* source = current;
        if (type == requested_type) {
            source = param_2;
            selected = current;
        }

        std::uint32_t destination = previous;

        if (type != 1) {
            if (type == 2) {
                destination = ((cursor + 0x9fu) & 0xffffff80u) - 0x20u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d822a(source, reinterpret_cast<void*>(destination));
                cursor = destination + field(source, 0x0c) + 0x20u;
            } else if (type == 4) {
                destination = ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d82ec(source, reinterpret_cast<void*>(destination));
                cursor = destination + field(source, 0x0c) + 0x20u;
            } else if (type == 16) {
                destination = ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d8466(source, reinterpret_cast<void*>(destination));
                cursor = destination + field(source, 0x0c) + 0x20u;
            } else if (type == 32) {
                destination = ((cursor + 0x1fu) & 0xfffffff0u) - 0x10u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d8619(source, reinterpret_cast<void*>(destination));
                cursor = destination + field(source, 0x0c) + 0x10u;
            } else if (type == 128) {
                destination = ((cursor + 0x1fu) & 0xfffffff0u) - 0x10u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d852e(source, reinterpret_cast<void*>(destination));
                cursor = destination + field(source, 0x0c) + 0x10u;
            } else {
                // Ghidra's pointer subtraction by 0x40 is a byte
                // subtraction by 0x100; its comparison is with zero.
                if (type != 0x100u)
                    return (type - 0x100u) & 0xffffff00u;

                destination = ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u;
                if (previous != 0)
                    FUN_006d6bf1(reinterpret_cast<void*>(previous), destination);
                FUN_006d83ab(source, reinterpret_cast<void*>(destination));
                const int size = FUN_006d8864(source);
                cursor = destination + static_cast<std::uint32_t>(size);
            }
        }

        previous = destination;

        if (type == requested_type) {
            const std::uint32_t next = field(selected, 0x04);
            current = next == 0
                ? nullptr
                : reinterpret_cast<void*>(
                    static_cast<std::uint32_t>(
                        reinterpret_cast<std::uintptr_t>(selected)) + next);
            selected = nullptr;
        } else {
            const std::uint32_t next = field(source, 0x04);
            current = next == 0
                ? nullptr
                : reinterpret_cast<void*>(
                    static_cast<std::uint32_t>(
                        reinterpret_cast<std::uintptr_t>(source)) + next);
        }
    }
}
