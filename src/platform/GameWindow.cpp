#include "platform/GameWindow.h"

namespace hp6::platform {

namespace {
constexpr char window_class_name[] = "HalfBloodPrinceMainWndClass";
constexpr char window_title[] = ""; // 0x0073a1cd begins with a zero byte.
}

bool register_game_window_class(HINSTANCE instance, WNDPROC procedure,
                                bool fullscreen) {
    UnregisterClassA(window_class_name, instance);
    WNDCLASSA definition{};
    definition.style = fullscreen ? 0x2020u : 0x2000u;
    if (!fullscreen) {
        definition.hCursor = LoadCursorA(nullptr, MAKEINTRESOURCEA(0x7f00));
    }
    definition.lpfnWndProc = procedure;
    definition.cbClsExtra = 0;
    definition.cbWndExtra = 0;
    definition.hInstance = instance;
    definition.hIcon = static_cast<HICON>(LoadImageA(
        nullptr, ".\\..\\hbp_icon.ico", IMAGE_ICON, 0x20, 0x20,
        LR_LOADFROMFILE));
    definition.hbrBackground = static_cast<HBRUSH>(GetStockObject(4));
    definition.lpszClassName = window_class_name;
    return RegisterClassA(&definition) != 0;
}

HWND create_game_window(HINSTANCE instance, int x, int y,
                        int width, int height, bool fullscreen) {
    RECT rect{x, y, x + width, y + height};
    DWORD extended_style = 0;
    DWORD style = 0x00cf0000u;
    HWND insert_after = nullptr;
    UINT position_flags = 3;
    if (fullscreen) {
        rect.left = 0;
        rect.top = 0;
        extended_style = 8;
        style = 0x80000000u;
        insert_after = HWND_TOPMOST;
        position_flags = 0x2003;
    }
    AdjustWindowRectEx(&rect, style, FALSE, extended_style);
    HWND window = CreateWindowExA(
        extended_style, window_class_name, window_title, style,
        rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, instance, nullptr);
    ShowWindow(window, SW_HIDE);
    SetWindowPos(window, insert_after, 0, 0, 0, 0, position_flags);
    if (fullscreen) {
        SetMenu(window, nullptr);
        SetThreadExecutionState(0x80000002u);
    }
    ShowWindow(window, SW_SHOW);
    return window;
}

}
