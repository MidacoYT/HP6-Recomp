void __cdecl FUN_006b227b(unsigned int param_1)
{
    void* const object = reinterpret_cast<void*>(FUN_006b18df(param_1, 1));
    const auto lock = reinterpret_cast<int>(object) + 0x28;
    int local_8;

    FUN_006d92a1(lock);

    // FUN_006b2147's declared return type is unresolved; this caller uses EAX
    // as a pointer, as do the neighboring callers.
    int* const result = reinterpret_cast<int*>(
        FUN_006b2147(object, param_1, &local_8));

    if (result != nullptr && local_8 != 0) {
        using Callback = void (__cdecl*)();
        auto* const vtable = *reinterpret_cast<Callback**>(result);
        vtable[3]();
    }

    FUN_006d92ac(lock);
}