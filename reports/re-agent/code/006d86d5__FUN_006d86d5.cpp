#include <cstdint>

void __thiscall FUN_006d86d5(void* this_, std::int32_t param_1)
{
    auto* source = static_cast<std::uint32_t*>(this_);
    std::uint32_t* previous = nullptr;
    std::uint32_t cursor = static_cast<std::uint32_t>(param_1);

    for (;;) {
        if (source == nullptr) {
            // The null write is present in the decompile.
            previous[1] = 0;
            return;
        }

        const int kind = FUN_006d6c09(source);
        std::uint32_t* output = previous;

        if (kind == 2) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x9fu) & 0xffffff80u) - 0x20u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d822a(source, output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + output[3] + 0x20u;
        } else if (kind == 4) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d82ec(source, output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + output[3] + 0x20u;
        } else if (kind == 0x10) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d8466(source, output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + output[3] + 0x20u;
        } else if (kind == 0x20) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x1fu) & 0xfffffff0u) - 0x10u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d8619(source, output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + output[3] + 0x10u;
        } else if (kind == 0x80) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x1fu) & 0xfffffff0u) - 0x10u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d852e(source, output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + output[3] + 0x10u;
        } else if (kind == 0x100) {
            output = reinterpret_cast<std::uint32_t*>(
                ((cursor + 0x2fu) & 0xfffffff0u) - 0x20u);
            if (previous != nullptr)
                FUN_006d6bf1(previous, reinterpret_cast<std::int32_t>(output));
            FUN_006d83ab(source, output);
            const int size = FUN_006d8864(output);
            cursor = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(output)) + size;
        }

        previous = output;
        const std::uint32_t nextOffset = source[1];
        source = nextOffset == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(
                  static_cast<std::uint32_t>(
                      reinterpret_cast<std::uintptr_t>(source)) + nextOffset);
    }
}