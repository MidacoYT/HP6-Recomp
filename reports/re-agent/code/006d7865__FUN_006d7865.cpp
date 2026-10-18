std::uint32_t* __cdecl FUN_006d7865(std::int32_t param_1, std::uint32_t param_2)
{
    auto* result = static_cast<std::uint32_t*>(FUN_006d6ab6(
        "cr_clip", 0x20, 0x10, 0x20, param_2, "shpcreate.cpp", 0x26f));

    FUN_006d7827(result, param_1);
    *result = (*result & 0xfffffbffu) | 0x400u;
    return result;
}