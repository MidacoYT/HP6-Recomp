bool __stdcall thunk_FUN_00ff8380()
{
    // The binary takes this argument from the incoming ESI register.
    HINSTANCE unaff_ESI;
    __asm mov unaff_ESI, esi

    UnregisterClassA("HalfBloodPrinceMainWndClass", unaff_ESI);

    WNDCLASSA wndClass; // The evidence identifies writes to only the first four fields.
    if (DAT_00866e2a == '\0') {
        wndClass.style = 0x2000;
        LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
    } else {
        wndClass.style = 0x2020;
    }

    wndClass.lpfnWndProc = FUN_005d8a30;
    wndClass.cbClsExtra = 0;
    wndClass.cbWndExtra = 0;

    LoadImageA(nullptr, ".\\..\\hbp_icon.ico", 1, 0x20, 0x20, 0x10);
    GetStockObject(4);

    return RegisterClassA(&wndClass) != 0;
}
