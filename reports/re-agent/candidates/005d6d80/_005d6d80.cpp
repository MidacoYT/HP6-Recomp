std::exception* __stdcall FUN_005d6d80(
    std::exception* param_1,
    std::string* param_2)
{
    ::new (static_cast<void*>(param_1)) std::exception;

    // The derived type and its member name are unresolved.
    *reinterpret_cast<void**>(param_1) =
        reinterpret_cast<void*>(0x0083e024);

    ::new (static_cast<void*>(
        reinterpret_cast<std::byte*>(param_1) + 0x0c))
        std::string(*param_2);

    return param_1;
}
