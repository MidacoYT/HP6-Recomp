#include <cstdint>

struct Entry_006d6ab6 {
    std::uint32_t index;
    std::uint32_t value;
    Entry_006d6ab6* next;
};

static_assert(sizeof(Entry_006d6ab6) == 12); // 32-bit target

// The type behind this global is unresolved; its first word points to the callback.
extern void** DAT_00895c84;

void __cdecl FUN_006d6ab6(
    std::uint32_t param_1, std::uint32_t param_2,
    std::uint32_t param_3, std::uint32_t param_4,
    std::uint32_t param_5, std::uint32_t param_6,
    std::uint32_t param_7)
{
    Entry_006d6ab6 entries[6] = {
        {6, param_7, &entries[1]},
        {5, param_6, &entries[2]},
        {4, param_5, &entries[3]},
        {3, param_4, &entries[4]},
        {2, param_3, &entries[5]},
        {1, param_1, nullptr}
    };

    using Callback = void (__cdecl *)(std::uint32_t, Entry_006d6ab6*);
    (*reinterpret_cast<Callback*>(*DAT_00895c84))(param_2, entries);
}
