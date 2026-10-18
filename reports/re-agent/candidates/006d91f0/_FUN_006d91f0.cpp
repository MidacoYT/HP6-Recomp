#include <atomic>
#include <cstdint>
#include <windows.h>

extern void __cdecl FUN_005dc710(std::int32_t);

void __cdecl FUN_006d91f0(std::uint32_t* param_1)
{
    const auto address = reinterpret_cast<std::int32_t>(param_1);

    if (address < 0x00896498 || address >= 0x00899698) {
        if (param_1 != nullptr) {
            if (*param_1 != 0) {
                CloseHandle(reinterpret_cast<HANDLE>(*param_1));
            }
            FUN_005dc710(address);
        }
    } else {
        const std::int32_t index = (address - 0x00896498) / 100;
        auto* slot = reinterpret_cast<std::uint32_t*>(
            0x008998c8 + index * 4);
        std::atomic_ref<std::uint32_t>(*slot).exchange(
            0, std::memory_order_seq_cst);

        if (*param_1 != 0) {
            CloseHandle(reinterpret_cast<HANDLE>(*param_1));
            return;
        }
    }
}
