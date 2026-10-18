std::uint32_t* __cdecl FUN_006d78f2(std::int32_t param_1, std::uint32_t param_2)
{
    const std::int32_t value = FUN_006d768d(param_1);
    auto* result = static_cast<std::uint32_t*>(FUN_006d6ab6(
        "cr_hots", value, 0x10, 0x20, param_2, "shpcreate.cpp", 0x29c));

    FUN_006d78a8(result, param_1);
    *result = (*result & 0xfffffbffu) | 0x400u;
    return result;
}
