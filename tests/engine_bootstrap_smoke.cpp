// Local behavior checks for FUN_005dcdf0 ("Engine_InitCoreSystemsAndMessages",
// called once from WinMain right after create_and_configure_render_device —
// see architecture/startup_path.md). Game-space symbols get test-local
// definitions, the way the other smoke tests already do.
#include "presentation/EngineBootstrap.h"
#include "presentation/TaskSystem.h"

#include <cstdint>
#include <cstdio>

// ---- hp6::presentation::allocate_tagged dependencies (service_holder) ----
void* PTR_FUN_0083e55c = nullptr;
void* PTR_FUN_0083e56c = nullptr;
void* DAT_00fcdd74 = nullptr;
void* DAT_00fcdd78 = nullptr;
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715da8 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

// ---- hp6::runtime::FrameClock dependencies (pulled in by
// initialize_frame_pump_timing) -------------------------------------------
std::uint8_t DAT_00cef72c = 0;
void* DAT_00fc7f90 = nullptr;
std::int32_t DAT_0089eb14 = 0;
std::int32_t DAT_00cef790 = 0;
std::uint32_t DAT_00d64570 = 0;
std::int32_t DAT_00d64574 = 0;
std::uint32_t DAT_00d64578 = 0;
std::int32_t DAT_00d6457c = 0;
std::int32_t DAT_00d644f0 = 0;
std::int32_t DAT_00d644f4 = 0;
float DAT_0080c680 = 1.0f;
float DAT_0083b230 = 1.0f;

// ---- EngineBootstrap.cpp's own extern dependencies ------------------------
std::int32_t DAT_00d63e80 = 0; // "not registered" sentinel

std::int32_t DAT_00d63eb0 = 0, DAT_00d63df8 = 0, DAT_00d63e30 = 0;
std::int32_t DAT_00d63ea0 = 0, DAT_00d63de4 = 0;

std::int32_t DAT_00d63ec0 = 0, DAT_00d63eb8 = 0, DAT_00d63e68 = 0, DAT_00d63ed0 = 0,
             DAT_00d63e58 = 0;
std::int32_t DAT_00d63e88 = 0, DAT_00d63e38 = 0, DAT_00d63e28 = 0, DAT_00d63ec8 = 0,
             DAT_00d63e90 = 0;
std::int32_t DAT_00d63e50 = 0, DAT_00d63ea8 = 0, DAT_00d63e40 = 0, DAT_00d63e18 = 0,
             DAT_00d63e98 = 0;
std::int32_t DAT_00d63e10 = 0, DAT_00d63e00 = 0, DAT_00d63e08 = 0;

std::int32_t DAT_00d63e70 = 0, DAT_00d63ed8 = 0, DAT_00d63e60 = 0;
std::uint32_t DAT_00cef884 = 0, DAT_00cef880 = 0;

std::uint32_t DAT_00fc8478 = 0;
void* PTR_PTR_008813c8 = nullptr;
std::uint8_t DAT_0087cc00[8] = {};

void* DAT_00cef730 = nullptr;

void* DAT_0089eb0c = nullptr;
void* PTR_PTR_0087eadc = nullptr;

void* DAT_00d6af14 = nullptr;

std::uint8_t DAT_00cef718 = 0, DAT_00cef69f = 0, DAT_00cef69e = 0;
float _DAT_008a4474 = 60.0f;

// ---- hp6::runtime::initialize_frame_pump_timing's own dependencies -------
float DAT_00ba9504 = 1000.0f;
float DAT_008482ec = 65536.0f;
float DAT_008003ac = 1.0f;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    PTR_FUN_0083e56c = reinterpret_cast<void*>(+[](std::int32_t size, const void*) -> void* {
        static std::uint8_t storage[64];
        (void)size;
        return storage;
    });

    initialize_engine_core_systems();

    expect("lifecycle_messages_registered", DAT_00d63eb0 != DAT_00d63e80);
    expect("delete_entity_registered", DAT_00d63df8 != DAT_00d63e80);
    expect("running_tick_registered", DAT_00d63e58 != DAT_00d63e80);
    expect("gsc_handles_copied", DAT_00d63e10 == DAT_00d63e90 && DAT_00d63e00 == DAT_00d63e50 &&
                                       DAT_00d63e08 == DAT_00d63ea8);
    expect("render_messages_registered", DAT_00d63e70 != DAT_00d63e80);
    expect("audio_hash_computed", DAT_00fc8478 != 0);
    expect("frame_pump_object_assigned", DAT_0089eb0c == &PTR_PTR_0087eadc);
    expect("time_manager_created", DAT_00cef730 != nullptr);
    expect("post_init_flags_set", DAT_00cef718 == 1 && DAT_00cef69f == 0 && DAT_00cef69e == 0);

    if (failures != 0) {
        std::printf("engine_bootstrap_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("engine_bootstrap_smoke: all checks passed\n");
    return 0;
}
