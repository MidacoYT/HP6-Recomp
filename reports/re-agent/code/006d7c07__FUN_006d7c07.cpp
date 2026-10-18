#include <cstdint>

void __thiscall FUN_006d7c07(std::uint32_t* self, std::uint32_t* param_1)
{
    std::uint32_t local_c4[48];
    FUN_006d73b8(local_c4);

    while (self != nullptr) {
        const std::uint32_t category = FUN_006d6c09(self);

        if (category == 2) {
            FUN_006d7a73(self, reinterpret_cast<int>(local_c4));
        } else if (category == 4) {
            FUN_006d7b14(self, reinterpret_cast<int>(local_c4));
        } else if (category == 0x10) {
            FUN_006d7b7a(self, reinterpret_cast<int>(local_c4));
        } else if (category == 0x20) {
            FUN_006d7bd2(self, reinterpret_cast<int>(local_c4));
        } else if (category == 0x80) {
            FUN_006d7baf(self, reinterpret_cast<int>(local_c4));
        } else if (category == 0x100) {
            FUN_006d7b49(self, reinterpret_cast<int>(local_c4));
        }

        const std::uint32_t next_offset = self[1]; // Unresolved field at +0x04.
        self = next_offset == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(
                  reinterpret_cast<std::uint8_t*>(self) + next_offset);
    }

    for (int i = 0; i < 48; ++i) {
        param_1[i] = local_c4[i];
    }
}