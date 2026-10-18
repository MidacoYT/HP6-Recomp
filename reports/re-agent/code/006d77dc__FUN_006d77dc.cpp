std::uint32_t* __cdecl FUN_006d77dc(std::int32_t param_1, std::uint32_t param_2)
{
    const std::int32_t value = FUN_006d760a(param_1);
    auto* result = static_cast<std::uint32_t*>(FUN_006d6ab6(
        "Created Clut Element",
        value,
        0x10,
        0x20,
        param_2,
        "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp",
        0x246));

    FUN_006d778a(result, param_1);
    *result = (*result & 0xfffffbffu) | 0x400u;
    return result;
}