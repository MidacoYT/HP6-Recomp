#include "platform/GameWindow.h"

namespace {
LRESULT CALLBACK test_window_procedure(HWND window, UINT message,
                                       WPARAM wparam, LPARAM lparam) {
    return DefWindowProcA(window, message, wparam, lparam);
}
}

int main() {
    const HINSTANCE instance = GetModuleHandleA(nullptr);
    if (!hp6::platform::register_game_window_class(
            instance, test_window_procedure, false)) return 1;
    WNDCLASSA registered{};
    if (!GetClassInfoA(instance, "HalfBloodPrinceMainWndClass", &registered) ||
        registered.style != 0x2000u ||
        registered.lpfnWndProc != test_window_procedure) return 2;
    HWND window = hp6::platform::create_game_window(
        instance, 5, 6, 320, 240, false);
    if (!IsWindow(window) ||
        (static_cast<DWORD>(GetWindowLongA(window, GWL_STYLE)) & 0x00ffffffu)
            != 0x00cf0000u) {
        return 3;
    }
    RECT client{};
    if (!GetClientRect(window, &client) ||
        client.right - client.left != 320 ||
        client.bottom - client.top != 240) return 4;
    DestroyWindow(window);
    if (!UnregisterClassA("HalfBloodPrinceMainWndClass", instance)) return 5;
    return 0;
}
