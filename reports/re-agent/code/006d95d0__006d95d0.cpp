std::uint32_t __cdecl FUN_006d95d0(
    std::uint8_t* param_1,
    std::uint32_t param_2,
    std::uint32_t param_3,
    std::uint32_t /* param_4 */,
    std::uint32_t param_5,
    int param_6)
{
    // The names and types of the fields at offsets +4 and +8 are unresolved.
    struct LocalStorage {
        std::uint8_t bytes[12];
        volatile std::int32_t local_10;
        std::uint32_t local_c;
        std::uint32_t local_8;
    } local;

    std::uint8_t* storage = param_1 ? param_1 : local.bytes;
    local.local_c = param_2;
    local.local_8 = param_3;
    local.local_10 = 0;

    const std::uintptr_t thread = _beginthreadex(
        nullptr, param_5, _StartAddress_006d9351,
        const_cast<std::int32_t*>(&local.local_10), 4,
        reinterpret_cast<unsigned*>(storage + 8));

    *reinterpret_cast<std::uintptr_t*>(storage + 4) = thread;
    if (thread == 0)
        return 0;

    FUN_006d94f5(reinterpret_cast<int>(storage), param_6);

    DWORD result = ResumeThread(*reinterpret_cast<HANDLE*>(storage + 4));
    while (local.local_10 == 0)
        result = SleepEx(1, 1);

    if (param_1 == nullptr)
        result = CloseHandle(*reinterpret_cast<HANDLE*>(storage + 4));

    return (result & 0xffffff00u) | 1u;
}