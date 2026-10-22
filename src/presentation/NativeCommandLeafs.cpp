#include <cstdint>
#include <cstdlib>
#include <windows.h>

extern std::int32_t DAT_00879654;

// Both verified PE images initialize these bytes to zero. The handler bodies
// below are disassembled from the SHA-verified unprotected HP6 x86 image.
void* DAT_00fcae18 = nullptr;
void* DAT_00fcae84 = nullptr;
std::uint8_t DAT_00cef76d = 0;

// 0x0045DF40: mov al,1; ret. Eight presentation commands share this target;
// command dispatch only uses the low byte of the return value.
std::int32_t command_return_one_0045df40(const char*) {
    return 1;
}

// 0x0048AD30..0x0048AD3C: storage TRC popup object, field +0x0C.
std::int32_t command_storage_popup_active_0048ad30(const char*) {
    const auto* popup = static_cast<const std::uint8_t*>(DAT_00fcae18);
    return *reinterpret_cast<const std::uint32_t*>(popup + 0xc) != 0;
}

// 0x0048B020..0x0048B031: toggle the byte and return success in AL.
std::int32_t command_toggle_text_clip_0048b020(const char*) {
    DAT_00cef76d = DAT_00cef76d == 0 ? 1 : 0;
    return 1;
}

// 0x0048B410/0x0048B430: update the pause-menu state only when it is 1.
// Both functions return AL=1 even when the menu pointer is null.
std::int32_t command_quit_game_0048b410(const char*) {
    if (DAT_00fcae84 != nullptr) {
        auto& state = *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uint8_t*>(DAT_00fcae84) + 0xc);
        if (state == 1) state = 2;
    }
    return 1;
}

std::int32_t command_resume_play_0048b430(const char*) {
    if (DAT_00fcae84 != nullptr) {
        auto& state = *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uint8_t*>(DAT_00fcae84) + 0xc);
        if (state == 1) state = 5;
    }
    return 1;
}

// 0x0048C130: IAT 0x007133A4 is USER32!PostQuitMessage.
std::int32_t command_quit_to_windows_0048c130(const char*) {
    PostQuitMessage(0);
    return 1;
}

// 0x0048C950: IAT 0x00713200 is MSVCR80!atoi. The original stores its
// result, including the zero result of an invalid string, without a clamp.
std::int32_t command_language_menu_select_0048c950(const char* argument) {
    DAT_00879654 = std::atoi(argument);
    return 1;
}
