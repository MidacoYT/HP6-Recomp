std::uint32_t* __thiscall FUN_006d8417(void* this_, std::uint32_t param_1)
{
    const int value = FUN_006d8864(reinterpret_cast<std::uint32_t*>(this_));
    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_clip", value, 0x10, 0x20, param_1,
                     "shpclone.cpp", 0x9f));

    FUN_006d83ab(this_, result);
    result[1] = 0;  // Member at offset 0x04; name unresolved.
    result[0] = (result[0] & 0xfffffbffu) | 0x400u;
    return result;
}
