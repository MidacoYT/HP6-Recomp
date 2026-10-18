std::uint32_t* __thiscall FUN_006d8816(void* self, std::uint32_t param_1)
{
    const std::uint32_t value = FUN_006d88d2(self);
    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_shp", value, 0x80, 0x20,
                     param_1, "shpclone.cpp", 0x1c4));

    FUN_006d86d5(self, reinterpret_cast<std::int32_t>(result));
    *result = (*result & 0xfffffbffu) | 0x400u;
    return result;
}