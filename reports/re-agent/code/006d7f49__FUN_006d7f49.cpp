std::uint32_t* __cdecl FUN_006d7f49(std::int32_t param_1, std::uint32_t param_2)
{
    const std::uint32_t size = FUN_006d76a6(param_1);
    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cr_shp", size, 0x80, 0x20, param_2,
                     "shpcreate.cpp", 0x311));

    FUN_006d7dea(result, param_1);
    *result = (*result & 0xfffffbffu) | 0x400u;
    return result;
}