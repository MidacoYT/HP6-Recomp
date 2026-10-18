// The list and node types are unresolved. Their observed offsets are kept explicit.
// This implementation assumes the target's 32-bit ABI.
void __thiscall FUN_006b2ce9(void* this_, int param_1, int param_2)
{
    if (param_1 == 0) {
        FUN_006b2c65(this_, param_2);
    } else {
        auto* const previous_link =
            reinterpret_cast<int*>(param_1 + 8);

        if (*previous_link == 0) {
            *reinterpret_cast<int*>(static_cast<char*>(this_) + 4) = param_2;
        }

        *reinterpret_cast<int*>(param_2 + 8) = *previous_link;
        *previous_link = param_2 + 8;

        auto* const count =
            reinterpret_cast<int*>(static_cast<char*>(this_) + 8);
        *count = *count + 1;
    }
}
