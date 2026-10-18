// The supplied evidence does not identify member names or the entry type.
// FUN_006b2147 is declared here with the pointer return used by this caller;
// its supplied decompilation incorrectly shows a void return.
using Callback = void (__cdecl *)(std::uint32_t, std::int32_t, std::uint32_t);

void __cdecl FUN_006b23b2(std::uint32_t param_1, Callback param_2)
{
    auto* object = reinterpret_cast<std::byte*>(FUN_006b18df(param_1, '\x01'));

    FUN_006d92a1(object + 0x28);

    std::int32_t local_8;
    auto* entry = reinterpret_cast<std::byte*>(
        FUN_006b2147(object, param_1, &local_8));

    if (local_8 == 0) {
        param_2(
            *reinterpret_cast<std::uint32_t*>(entry + 0x0c),
            *reinterpret_cast<std::int8_t*>(entry + 0x11),
            *reinterpret_cast<std::uint32_t*>(entry + 0x18));
    } else {
        *reinterpret_cast<Callback*>(entry + 0x1c) = param_2;
    }

    FUN_006d92ac(object + 0x28);
}
