#pragma once
#include <windows.h>

namespace hp6::platform {

bool register_game_window_class(HINSTANCE instance, WNDPROC procedure,
                                bool fullscreen); // 0x00ff8380

// The binary receives x in ECX and y in EAX; the reconstructed interface
// makes both coordinates explicit.
HWND create_game_window(HINSTANCE instance, int x, int y,
                        int width, int height, bool fullscreen); // 0x005d8ed0

}
