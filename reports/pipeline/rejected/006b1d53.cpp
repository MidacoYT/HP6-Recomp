undefined4 __stdcall FUN_006b1d53()
{
    FUN_006d92a1(reinterpret_cast<int>(DAT_00892b60 + 3));

    int* const savedPointer = DAT_00892b60;
    const undefined4 result = FUN_006b2cbd(DAT_00892b60);

    FUN_006d92ac(reinterpret_cast<int>(savedPointer + 3));
    return result;
}
