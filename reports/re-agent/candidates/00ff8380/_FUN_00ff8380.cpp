#include <windows.h>

extern unsigned char DAT_00866e2a;
extern LRESULT CALLBACK FUN_005d8a30(HWND, UINT, WPARAM, LPARAM);

bool __stdcall FUN_00ff8380(void)
{
    // The decompile shows ESI as a live-in value, but does not identify
    // its source. This preserves that register dependency on 32-bit MSVC.
    HINSTANCE instance;
    __asm mov instance, esi

    constexpr char className[] = "HalfBloodPrinceMainWndClass";
    UnregisterClassA(className, instance);

    WNDCLASSA windowClass{};
    if (DAT_00866e2a == 0) {
        windowClass.style = 0x2000;
        windowClass.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
    } else {
        windowClass.style = 0x2020;
    }

    windowClass.lpfnWndProc = FUN_005d8a30;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = instance;
    windowClass.hIcon = static_cast<HICON>(
        LoadImageA(nullptr, ".\\..\\hbp_icon.ico", IMAGE_ICON,
                   0x20, 0x20, LR_LOADFROMFILE));
    windowClass.hbrBackground = static_cast<HBRUSH>(GetStockObject(4));
    windowClass.lpszClassName = className;

    return RegisterClassA(&windowClass) != 0;
}
