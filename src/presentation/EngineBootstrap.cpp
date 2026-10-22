#include "presentation/EngineBootstrap.h"
#include "presentation/TaskSystem.h"
#include "runtime/FrameClock.h"

#include <cstdint>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::int32_t DAT_00d63e80; // "not registered" sentinel shared by every message slot

extern std::int32_t DAT_00d63eb0, DAT_00d63df8, DAT_00d63e30; // FUN_005dcdf0's own 3 messages
extern std::int32_t DAT_00d63ea0, DAT_00d63de4;

extern std::int32_t DAT_00d63ec0, DAT_00d63eb8, DAT_00d63e68, DAT_00d63ed0, DAT_00d63e58;
extern std::int32_t DAT_00d63e88, DAT_00d63e38, DAT_00d63e28, DAT_00d63ec8, DAT_00d63e90;
extern std::int32_t DAT_00d63e50, DAT_00d63ea8, DAT_00d63e40, DAT_00d63e18, DAT_00d63e98;
extern std::int32_t DAT_00d63e10, DAT_00d63e00, DAT_00d63e08;

extern std::int32_t DAT_00d63e70, DAT_00d63ed8, DAT_00d63e60;
extern std::uint32_t DAT_00cef884, DAT_00cef880;

extern std::uint32_t DAT_00fc8478; // "SSGAudio" running hash
extern void* PTR_PTR_008813c8;     // engine named-subsystem registry (deferred vtable call)
extern std::uint8_t DAT_0087cc00[8];

extern void* DAT_00cef730; // TimeManager::Instance singleton (see FrameClock.cpp)

extern void* DAT_0089eb0c;       // frame pump "this" (see FrameClock.cpp)
extern void* PTR_PTR_0087eadc;   // first interface of the engine object (0x0083E5C8)

extern void* DAT_00d6af14; // engine-service object; see DeviceCreate.h's doc comment

extern std::uint8_t DAT_00cef718, DAT_00cef69f, DAT_00cef69e;
extern float _DAT_008a4474; // also read/written during device creation; see DeviceCreate.cpp

namespace hp6::presentation {

void register_named_message_handler(void** slot, const char* name, void* callback) {
    (void)callback; // never invoked: nothing integrated so far emits these messages
    if (name == nullptr || *name == '\0') {
        *slot = reinterpret_cast<void*>(static_cast<std::intptr_t>(DAT_00d63e80));
        return;
    }
    // One heap-allocated sentinel per call, intentionally never freed —
    // matching the real registry's own lifetime (it lives for the whole
    // process, like every other lazy-init singleton in this project).
    *slot = new std::uint32_t(0);
}

void unregister_named_message_handler(void* /*handle*/) {}

void register_lifecycle_messages() {
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ec0), "iMsgSetRunningMode", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63eb8), "iMsgSetPausedMode", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e68), "iMsgSetFrozenMode", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ed0), "iMsgRunningPreTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e58), "iMsgRunningTick", reinterpret_cast<void*>(0x83e4ec));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e88), "iMsgRunningPostTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e38), "iMsgPausedPreTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e28), "iMsgPausedTick", reinterpret_cast<void*>(0x83e4ec));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ec8), "iMsgPausedPostTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e90), "iMsgGSCRunningPreTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e50), "iMsgGSCRunningTick", reinterpret_cast<void*>(0x83e4ec));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ea8), "iMsgGSCRunningPostTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e40), "iMsgGSCPausedPreTick", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e18), "iMsgGSCPausedTick", reinterpret_cast<void*>(0x83e4ec));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e98), "iMsgGSCPausedPostTick", nullptr);
}

void copy_gsc_message_handles() {
    if (DAT_00d63ec0 != DAT_00d63e80) {
        unregister_named_message_handler(reinterpret_cast<void*>(static_cast<std::intptr_t>(DAT_00d63ec0)));
    }
    DAT_00d63e10 = DAT_00d63e90;
    DAT_00d63e00 = DAT_00d63e50;
    DAT_00d63e08 = DAT_00d63ea8;
}

void register_render_messages() {
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e70), "iMsgDoRender", reinterpret_cast<void*>(0x83e4f8));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ed8), "iMsgDoRenderDirectorsCamera", reinterpret_cast<void*>(0x83e4f8));
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e60), "iMsgPreShowRaster", reinterpret_cast<void*>(0x83e4f8));
    DAT_00cef884 = DAT_00cef880;
}

void register_debug_audio_subsystem() {
    const char* name = "SSGAudio";
    DAT_00fc8478 = 0x1505;
    char c = *name;
    const char* p = name;
    do {
        const int ch = static_cast<unsigned char>(c);
        c = p[1];
        ++p;
        DAT_00fc8478 = static_cast<std::uint32_t>(ch) + DAT_00fc8478 * 0x21u;
    } while (c != '\0');

    // 0x008813c8's vtable slot 0 — NOT YET INTEGRATED, documented no-op.
    (void)PTR_PTR_008813c8;
    (void)DAT_0087cc00;
}

bool is_debug_build_overlay_enabled() {
    return false; // this is a FINAL build, matching every path this project exercises
}

void show_debug_build_overlay_once() {
    if (!is_debug_build_overlay_enabled()) {
        return; // dead in every path this project currently exercises
    }
    // NOT YET INTEGRATED: builds and would display a debug overlay
    // string ("Build: FINAL on Jan 15 2009 at 14:31:50", resolution,
    // format, per-mode capability strings). Deferred: unreachable while
    // is_debug_build_overlay_enabled() returns false.
}

void* construct_time_manager(void* instance) {
    return instance; // trivial pass-through; see header doc comment
}

void* get_or_create_time_manager() {
    if (DAT_00cef730 == nullptr) {
        void* instance = hp6::presentation::allocate_tagged(8, "TimeManager::Instance");
        DAT_00cef730 = (instance != nullptr) ? construct_time_manager(instance) : nullptr;
    }
    // NOT YET INTEGRATED: 0x00ff7da0 additionally calls several more
    // not-yet-traced engine setup functions (thunk_FUN_00ff7cb0,
    // FUN_0068bc60/bdd0, thunk_FUN_00ff4530, a
    // "GameServices::CCTDataManager" allocation) at this same point —
    // deferred as a documented no-op, since this function's only
    // observable contract this project currently depends on is that
    // DAT_00cef730 ends up non-null.
    return DAT_00cef730;
}

namespace {
// 0x00d6af14's vtable slot 2 — NOT YET INTEGRATED. See DeviceCreate.cpp's
// own placeholder for slot 0 of the same object; this is the symmetric
// no-op for the slot FUN_005dcdf0 itself calls.
void invoke_engine_service_slot2(void*) {}
} // namespace

void initialize_engine_core_systems() {
    register_debug_audio_subsystem();
    hp6::runtime::initialize_frame_pump_timing(_DAT_008a4474);
    DAT_0089eb0c = &PTR_PTR_0087eadc;

    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63eb0), "iMsgDeleteEventHandler", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63df8), "iMsgDeleteEntity", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63e30), "iMsgOnDeleteEntity", reinterpret_cast<void*>(0x83e2b0));

    register_lifecycle_messages();
    copy_gsc_message_handles();
    register_render_messages();
    show_debug_build_overlay_once();
    show_debug_build_overlay_once();

    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63ea0), "iMsgStartSystem", nullptr);
    register_named_message_handler(reinterpret_cast<void**>(&DAT_00d63de4), "iMsgStopSystem", nullptr);

    invoke_engine_service_slot2(DAT_00d6af14);

    get_or_create_time_manager();
    get_or_create_time_manager();

    DAT_00cef718 = 1;
    DAT_00cef69f = 0;
    DAT_00cef69e = 0;
}

} // namespace hp6::presentation
