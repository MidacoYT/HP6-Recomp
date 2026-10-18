int __cdecl FUN_006d9d15(int param_1)
{
    if (DAT_00899d5c == 0) {
        FUN_006b3b0e(&DAT_00899d60, 0x20);
        FUN_006b3b0e(&hTimer_008527e0, 0x18);
        DAT_00899d88 = 0;
        DAT_00899d5c = 1;
    }

    DAT_00899d84 = 0;
    hTimer_008527e0 = CreateWaitableTimerA(nullptr, TRUE, nullptr);

    // These adjacent globals hold the low and high words of the LARGE_INTEGER.
    const auto dueTime = static_cast<std::int32_t>(
        -10000000LL / static_cast<std::int64_t>(param_1));
    lpDueTime_008527f0 =
        reinterpret_cast<LARGE_INTEGER*>(static_cast<std::intptr_t>(dueTime));
    _DAT_008527f4 = dueTime >> 31;

    FUN_006d95d0(&DAT_008527e4, &LAB_006d9cae, 0, 0, 0, 2);
    DAT_00899d80 = param_1;

    SetWaitableTimer(
        hTimer_008527e0,
        reinterpret_cast<LARGE_INTEGER*>(&lpDueTime_008527f0),
        0, nullptr, nullptr, FALSE);

    return DAT_00899d80;
}
