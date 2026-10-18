HWND __thiscall FUN_005d8ed0(
    void* this_, HINSTANCE instance, int width, int height)
{
    // The caller supplies the vertical position in EAX.
    LONG y;
    __asm mov y, eax

    const LONG x = reinterpret_cast<LONG>(this_);
    RECT rect;
    rect.right = x + width;
    rect.bottom = y + height;

    DWORD exStyle = 0;
    HWND insertAfter = nullptr;
    DWORD style;
    UINT flags;

    if (DAT_00866e2a == '\0') {
        style = reinterpret_cast<DWORD>(&dwStyle_00cf0000);
        flags = 3;
        rect.left = x;
        rect.top = y;
    } else {
        style = 0x80000000;
        flags = 0x2003;
        rect.left = 0;
        rect.top = 0;
        insertAfter = reinterpret_cast<HWND>(0xffffffff);
        exStyle = 8;
    }

    AdjustWindowRectEx(&rect, style, FALSE, exStyle);

    HWND window = CreateWindowExA(
        exStyle,
        "HalfBloodPrinceMainWndClass",
        reinterpret_cast<LPCSTR>(&lpWindowName_0073a1cd),
        style,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        nullptr,
        nullptr,
        instance,
        nullptr);

    ShowWindow(window, 0);
    SetWindowPos(window, insertAfter, 0, 0, 0, 0, flags);

    if (DAT_00866e2a != '\0') {
        SetMenu(window, nullptr);
        SetThreadExecutionState(0x80000002);
    }

    ShowWindow(window, 5);
    return window;
}