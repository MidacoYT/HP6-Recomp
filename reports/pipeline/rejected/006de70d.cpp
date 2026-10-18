std::uint32_t* __thiscall FUN_006de70d(void* this_, std::uint8_t param_1)
{
    *reinterpret_cast<void***>(this_) = &PTR_FUN_0071b62c;
    FUN_006de7d0(this_);

    if ((param_1 & 1) != 0) {
        FUN_006dcbc7(this_, 0x78);
    }

    return static_cast<std::uint32_t*>(this_);
}
