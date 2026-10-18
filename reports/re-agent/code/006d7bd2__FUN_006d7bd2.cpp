// The member names and containing types for offsets +0x08, +0x04,
// and +0x90 are unresolved in the supplied evidence.
void __thiscall FUN_006d7bd2(void* this_, int param_1)
{
    FUN_006d7a48(this_, param_1);

    auto* output = reinterpret_cast<std::uint8_t*>(param_1);
    *reinterpret_cast<std::uint32_t*>(output + 0x04) |= 0x20u;

    auto* text = *reinterpret_cast<char**>(
        static_cast<std::uint8_t*>(this_) + 0x08);
    if (text != nullptr) {
        text = reinterpret_cast<char*>(
            reinterpret_cast<std::uintptr_t>(text) +
            reinterpret_cast<std::uintptr_t>(this_));
    }

    char* const after_first = text + 1;
    char ch;
    do {
        ch = *text;
        ++text;
    } while (ch != '\0');

    *reinterpret_cast<std::int32_t*>(output + 0x90) =
        static_cast<std::int32_t>(
            reinterpret_cast<std::uintptr_t>(text) -
            reinterpret_cast<std::uintptr_t>(after_first));
}