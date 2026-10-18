#include <cstdint>

// The type of the object referenced by this global remains unresolved.
// Its first 32-bit word points to a table of function pointers.
extern std::uint32_t* DAT_0089adcc;

void __cdecl FUN_006dcbc7(std::uint32_t param_1, std::uint32_t param_2)
{
    using Function = void (__cdecl*)(std::uint32_t, std::uint32_t);

    const auto table = *DAT_0089adcc;
    const auto function = *reinterpret_cast<Function*>(table + 4);
    function(param_1, param_2);
}