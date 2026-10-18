// The supplied evidence does not identify the types or names of these fields.
void __thiscall FUN_006d7baf(void* this_, std::int32_t param_1)
{
    FUN_006d7a48(this_, param_1);

    auto* destination = reinterpret_cast<std::uint8_t*>(param_1);
    auto* source = static_cast<std::uint8_t*>(this_);

    destination[0x04] |= 0x80;
    *reinterpret_cast<std::uint32_t*>(destination + 0x84) =
        *reinterpret_cast<std::uint32_t*>(source + 0x0c);
}
