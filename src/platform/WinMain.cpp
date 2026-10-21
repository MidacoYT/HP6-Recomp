#include "platform/WinMain.h"

#include "platform/EngineSubsystems.h"
#include "platform/GameWindow.h"
#include "platform/HandleRegistry.h"
#include "platform/MessageLoop.h"
#include "platform/RegistryConfig.h"
#include "platform/StartupSupport.h"
#include "platform/StorageInit.h"
#include "presentation/BootupFlow.h"
#include "presentation/EngineBootstrap.h"
#include "render/DeviceCreate.h"
#include "render/DeviceLostHandler.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_00866e2a; // "engine is actually running" flag (also in MessageLoop.cpp)

extern std::int32_t DAT_00cf1968; // Width
extern std::int32_t DAT_00cf196c; // BitDepth
extern std::int32_t DAT_00cf1970; // ShadowLOD
extern std::int32_t DAT_00cf197c; // MaxFarClip
extern std::int32_t DAT_00cf1980; // ParticleRate
extern std::int32_t DAT_00cf1988; // ParticleCullDistance
extern std::int32_t DAT_00cf1984; // CullDistance
extern std::int32_t DAT_00cf198c; // DisableFog
extern std::int32_t DAT_00cf1990; // DisablePostPro
extern std::int32_t DAT_00cf1994; // FilterFlip
extern std::int32_t DAT_00cf1998; // AAMode (also read by DeviceCreate.cpp)
extern std::int32_t DAT_00cf199c; // UseAdditionalModes
extern std::int32_t DAT_0096b468[6]; // Mode%dWidth
extern std::int32_t DAT_00a70860[6]; // Mode%dHeight
extern std::int32_t DAT_00cf19a4;    // OptionResolution (also in RegistryConfig.cpp)
extern std::int32_t DAT_008663b8;    // OptionLOD fallback (also in RegistryConfig.cpp)
extern std::int32_t DAT_008663c0;    // OptionBrightness (also in RegistryConfig.cpp)
extern std::int32_t DAT_00cf1974;    // MaxTextureSize
extern std::int32_t DAT_00cf1978;    // MaxShadowTextureSize

extern float DAT_008663a8;        // cached aspect ratio (confirmed dead-branch use only)
extern float DAT_007289d4;        // fixed aspect-ratio constant
extern std::int32_t DAT_00cf1950; // fullscreen/custom-resolution-list flag (also in DeviceCreate.cpp)

extern void* DAT_00cef694;       // the game window handle
extern void* DAT_00cef698;       // bootup-service object (documented deferred nullptr)
extern std::uint8_t DAT_00cef68e; // set once right after trigger_bootup_flow
extern std::uint8_t DAT_0089c427; // set once right before the engine bootstrap
extern std::uint8_t DAT_00cef72c; // cleared right after the message loop returns (also in MessageLoop.cpp)
extern std::uint8_t DAT_00cef68f; // "a fatal error was already shown" flag (also in MessageLoop.cpp)
extern std::int32_t DAT_00cef55c; // startup-grace-period counter (also in MessageLoop.cpp)
extern float DAT_00ba9504;        // interval numerator (also in FrameClock.cpp)

namespace hp6::platform {
namespace {

constexpr char kAppName[] = "Harry Potter and the Half Blood Prince";
constexpr char kSection[] = "GameSettings";
constexpr char kWindowClassName[] = "HalfBloodPrinceMainWndClass";

std::int32_t read_setting(const char* key, std::int32_t default_value) {
    return read_int_setting(key, kSection, kAppName, default_value);
}

std::string read_string(const char* key, const char* default_value) {
    return read_string_setting(key, kAppName, kSection, default_value);
}

bool real_system_parameters_call(std::uint32_t action, std::uint32_t size, void* data,
                                 std::uint32_t options) {
    return SystemParametersInfoA(action, size, data, options) != 0;
}

// 0x005d8a30. Only WM_DESTROY is reconstructed for real: it is the one
// case required for a real window's close button to make
// run_message_loop actually quit. Every other case (WM_ACTIVATE,
// WM_SYSCOMMAND screensaver-blocking, WM_SETCURSOR, ...) is a
// documented deferred DefWindowProcA passthrough.
LRESULT CALLBACK game_window_procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(window, message, wparam, lparam);
}

// NOT YET INTEGRATED: the real factory call behind WinMain's
// DAT_00cef698 is a double-indirected call through the service
// holder's second slot ((**(code**)holder[1])(resource_size,
// &descriptor)); its target function and the {tag, size}-shaped
// descriptor it is given are not yet traced. Deferred as a documented
// nullptr — every later use of this object (the Release-on-shutdown
// block below and hp6::platform::initialize_file_system_storage's own
// gate on it) stays honestly unreachable as a result, matching the
// exported x86's own behaviour whenever this factory fails.
void* create_bootup_service_object() {
    service_holder();
    const std::int32_t resource_size = startup_resource_size(8, 0x10);
    (void)resource_size;
    return nullptr;
}

} // namespace

void real_terminate_process(void* process, std::uint32_t exit_code) {
#ifdef _WIN32
    TerminateProcess(static_cast<HANDLE>(process), exit_code);
#else
    (void)process;
    (void)exit_code;
#endif
}

std::int32_t run_win_main(HINSTANCE instance, char* command_line, ProcessTerminationCall terminate) {
    _control87(0x20000, 0x30000);

    char module_path[0x104] = {};
    if (GetModuleFileNameA(nullptr, module_path, sizeof(module_path)) != 0) {
        if (char* last_slash = std::strrchr(module_path, '\\')) {
            *last_slash = '\0';
            SetCurrentDirectoryA(module_path);
        }
    }

    SystemParameterBuffers saved{};
    real_system_parameters_call(0x3a, 8, saved.first.data(), 0);
    real_system_parameters_call(0x34, 8, saved.second.data(), 0);
    real_system_parameters_call(0x32, 0x18, saved.third.data(), 0);
    apply_system_parameters(false, saved, &real_system_parameters_call);

    parse_command_line_overrides(command_line);

    std::int32_t width = 0;
    DAT_00cef694 = nullptr;

    if (FindWindowA(kWindowClassName, nullptr) == nullptr) {
        if (register_game_window_class(instance, &game_window_procedure, DAT_00866e2a != 0)) {
            width = 0x280;
            std::int32_t height = 0x1e0;
            std::int32_t pos_x = 0;
            std::int32_t pos_y = 0;
            bool maximized = false;
            bool minimized = false;

            DAT_00cf1968 = read_setting("Width", 0);
            if (DAT_00cf1968 != 0) {
                DAT_00866e2a = 1;
            }
            DAT_00cf196c = read_setting("BitDepth", 0);
            DAT_00cf1970 = read_setting("ShadowLOD", 0);
            DAT_00cf197c = read_setting("MaxFarClip", 0);
            DAT_00cf1980 = read_setting("ParticleRate", 0);
            DAT_00cf1988 = read_setting("ParticleCullDistance", 0);
            DAT_00cf1984 = read_setting("CullDistance", 0);
            DAT_00cf198c = read_setting("DisableFog", 0);
            DAT_00cf1990 = read_setting("DisablePostPro", 0);
            DAT_00cf1994 = read_setting("FilterFlip", 0);
            DAT_00cf1998 = read_setting("AAMode", 0);
            DAT_00cf199c = read_setting("UseAdditionalModes", 0);

            for (int mode = 0; mode < 6; ++mode) {
                char key[32];
                std::snprintf(key, sizeof(key), "Mode%dWidth", mode);
                DAT_0096b468[mode] = read_setting(key, 0);
                std::snprintf(key, sizeof(key), "Mode%dHeight", mode);
                DAT_00a70860[mode] = read_setting(key, 0);
            }

            DAT_00cf19a4 = read_setting("OptionResolution", 0);
            DAT_008663b8 = read_setting("OptionLOD", 1);
            DAT_008663c0 = read_setting("OptionBrightness", 5);
            DAT_00cf1974 = read_setting("MaxTextureSize", 0);
            DAT_00cf1978 = read_setting("MaxShadowTextureSize", 0);
            DAT_008663a8 = DAT_007289d4;

            if (DAT_00866e2a == 0) {
                pos_x = parse_setting_string_to_int(read_string("PosX", "300"));
                pos_y = parse_setting_string_to_int(read_string("PosY", "32"));
                if (DAT_00cf1968 == 0) {
                    width = parse_setting_string_to_int(read_string("SizeX", "640"));
                    height = parse_setting_string_to_int(read_string("SizeY", "480"));
                }
                // else: confirmed unreachable here (DAT_00866e2a == 0
                // implies DAT_00cf1968 == 0, since the only write to
                // DAT_00866e2a above is conditioned on DAT_00cf1968 !=
                // 0) — the exported x86's own dead branch (deriving a
                // custom height from Width via the DAT_008663a8 aspect
                // ratio) is not reproduced.
                maximized = read_string("Maximized", "false") == "true";
                minimized = read_string("Minimized", "false") == "true";
            } else {
                DAT_00cf1950 = 1;
                pos_x = 0;
                pos_y = 0;
            }

            DAT_00cef694 =
                create_game_window(instance, pos_x, pos_y, width, height, DAT_00866e2a != 0);

            if (DAT_00cef694 != nullptr) {
                RECT client{};
                GetClientRect(static_cast<HWND>(DAT_00cef694), &client);

                DAT_00cef698 = create_bootup_service_object();

                hp6::render::create_and_configure_render_device(
                    static_cast<std::uint32_t>(client.right - client.left),
                    static_cast<std::uint32_t>(client.bottom - client.top));
                initialize_file_system_storage();
                reset_particle_system_tables();
                initialize_debug_variable_system();
                initialize_runtime_services();
                DAT_0089c427 = 1;
                DAT_00ba9504 = 2.0f;
                hp6::presentation::initialize_engine_core_systems();

                if (is_platform_compatible()) {
                    hp6::presentation::trigger_bootup_flow(nullptr);
                    DAT_00cef68e = 1;

                    if (DAT_00866e2a == 0) {
                        if (!maximized) {
                            if (!minimized) {
                                int show_command = SW_SHOWNORMAL;
                                if (read_string("Maximized", "false") == "true") {
                                    show_command = SW_MAXIMIZE;
                                } else if (read_string("Minimized", "false") == "true") {
                                    show_command = SW_MINIMIZE;
                                }
                                ShowWindow(static_cast<HWND>(DAT_00cef694), show_command);
                            } else {
                                ShowWindow(static_cast<HWND>(DAT_00cef694), SW_MINIMIZE);
                            }
                        } else {
                            ShowWindow(static_cast<HWND>(DAT_00cef694), SW_MAXIMIZE);
                        }
                    }
                    UpdateWindow(static_cast<HWND>(DAT_00cef694));

                    const std::int32_t message_loop_result = run_message_loop();
                    hp6::render::run_post_message_loop_cleanup_primary(0);
                    DAT_00cef72c = 0;
                    hp6::render::run_post_message_loop_cleanup_secondary();
                    hp6::render::shutdown_render_device();
                    release_cached_debug_strings();
                    // NOT YET INTEGRATED: a DAT_00cef698-gated release
                    // block, confirmed unreachable (create_bootup_
                    // service_object always returns nullptr here).
                    save_display_options();
                    // NOT YET INTEGRATED: a DAT_00fcaf34-gated cleanup
                    // block, confirmed unreachable (nothing this
                    // project has integrated ever sets DAT_00fcaf34).
                    // NOT YET INTEGRATED: FUN_006599b0(), documented
                    // deferred no-op.
                    for (int cursor_count = 0; cursor_count < 1;) {
                        cursor_count = ShowCursor(TRUE);
                    }
                    DAT_00cef68f = 1;
                    if (DAT_00cef55c == 0) {
                        // NOT YET INTEGRATED: thunk_FUN_0100ea00(),
                        // documented deferred no-op (see MessageLoop.cpp).
                    }
                    DAT_00cef55c += 1;

                    apply_system_parameters(true, saved, &real_system_parameters_call);
                    terminate(GetCurrentProcess(), 1);
                    // Unlike the exported x86 (which never returns past
                    // a successful TerminateProcess), a test-injected
                    // `terminate` may choose not to end the process; in
                    // that case, return the message loop's own result
                    // so the test can observe it.
                    return message_loop_result;
                }
            }
        }
    } else {
        terminate(GetCurrentProcess(), 0);
        return 0;
    }

    if (IsWindow(static_cast<HWND>(DAT_00cef694))) {
        DestroyWindow(static_cast<HWND>(DAT_00cef694));
        run_message_loop();
    }
    real_system_parameters_call(0x3b, 8, saved.first.data(), 0);
    real_system_parameters_call(0x35, 8, saved.second.data(), 0);
    real_system_parameters_call(0x33, 0x18, saved.third.data(), 0);
    return width;
}

} // namespace hp6::platform

// FUN_005d9300 is called with the WinMain ABI by the original
// ___tmainCRTStartup at 0x006b9457. MSVC supplies the CRT startup for this
// target; this symbol maps its four WinMain arguments to the reconstructed
// body above. The decompiled body does not use hPrevInstance.
int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR command_line, int) {
    return hp6::platform::run_win_main(instance, command_line);
}
