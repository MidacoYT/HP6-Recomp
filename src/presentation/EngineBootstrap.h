#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x005dae30. NOT YET INTEGRATED (real behavior): the engine's named
// message-handler registry. The real function is a hash-table/string-
// interning-backed registry (calls FUN_005dffc0, FUN_005df8f0,
// FUN_005dafa0, none traced — a full message-bus subsystem in its own
// right) that every one of this project's ~20 current call sites only
// needs to produce a stable, non-null `*slot` from (an engine-wide
// sentinel equal to "not yet registered"). This stub allocates one
// 4-byte sentinel per call and stores its address in `*slot`, without
// implementing the real registry or ever invoking `callback` — nothing
// integrated so far emits any of these named messages, so the callback
// is never observably needed yet.
void register_named_message_handler(void** slot, const char* name, void* callback);

// 0x005db240. NOT YET INTEGRATED: unregisters a handler created by
// register_named_message_handler. A documented no-op placeholder,
// consistent with that function's own stub nature.
void unregister_named_message_handler(void* handle);

// 0x00ff8de0. Registers the 15 "mode"/"tick" lifecycle messages
// (iMsgSetRunningMode, iMsgRunningTick, iMsgGSCPausedPostTick, etc.)
// through register_named_message_handler.
void register_lifecycle_messages();

// 0x0100e9b0 (reached through the thunk at 0x005dca90). Unregisters the
// iMsgSetRunningMode handle if it was ever registered, then copies three
// of the handles register_lifecycle_messages just produced
// (iMsgGSCRunningPreTick/Tick/PostTick) into three other slots.
void copy_gsc_message_handles();

// 0x00ff8f00 (reached through the thunk at 0x005dcfd0). Registers the 3
// render-pass messages (iMsgDoRender, iMsgDoRenderDirectorsCamera,
// iMsgPreShowRaster) and copies DAT_00cef880 into DAT_00cef884.
void register_render_messages();

// 0x005a1cd0. Hashes the literal string "SSGAudio" into DAT_00fc8478
// (a simple *33+char running hash, matching the exported x86 loop
// exactly) and registers it with the engine's named-subsystem registry
// through a virtual call on PTR_PTR_008813c8 — NOT YET INTEGRATED, the
// vtable call is a documented no-op placeholder.
void register_debug_audio_subsystem();

// 0x01002c80. NOT YET INTEGRATED: the real function decides whether this
// is a non-final/debug build that should show the build-info overlay.
// This stub always returns false (matching a FINAL build), so
// show_debug_build_overlay_once's body below is dead code in every path
// this project currently exercises — not a fabrication of the real
// flag's logic, just its FINAL-build value.
bool is_debug_build_overlay_enabled();

// 0x005dd020 (called twice from FUN_005dcdf0). The first call (guarded by
// DAT_00cef68f) builds and would display a debug overlay string with the
// build date/time ("Build: FINAL on Jan 15 2009 at 14:31:50" — the
// literal build timestamp embedded in the exported x86), the active
// resolution/format, and per-mode capability strings, only when
// is_debug_build_overlay_enabled() is true. Already a no-op in every
// path this project currently exercises, since that flag is false.
void show_debug_build_overlay_once();

// 0x00ffa270 (reached through the thunk at the call site inside
// get_or_create_time_manager). NOT YET INTEGRATED: the real
// TimeManager constructor. A trivial pass-through stub (the object was
// already zero-allocated by hp6::presentation::allocate_tagged), matching
// this project's house style for not-yet-traced trivial-looking
// constructors (compare hp6::render's handling of FUN_00401220).
void* construct_time_manager(void* instance);

// 0x00ff7da0 / 0x00ff7e80 (two call sites, same lazy-init shape). Creates
// the engine's TimeManager singleton (DAT_00cef730, already read by
// hp6::runtime::accumulate_simulation_time_sample) the first time either
// is called, through hp6::presentation::allocate_tagged(8,
// "TimeManager::Instance") + construct_time_manager. 0x00ff7da0 goes on
// to call several more not-yet-traced engine setup functions
// (thunk_FUN_00ff7cb0, FUN_0068bc60/bdd0, thunk_FUN_00ff4530, a
// "GameServices::CCTDataManager" allocation) — deferred as documented
// no-op placeholders, since get_or_create_time_manager's only
// observable contract this project currently depends on is that
// DAT_00cef730 ends up non-null.
void* get_or_create_time_manager();

// 0x005dcdf0 ("Engine_InitCoreSystemsAndMessages" per architecture/
// startup_path.md, cross-referenced against the independent IDA
// knowledge base). Called once from WinMain (0x005d9300), right after
// create_and_configure_render_device. Seeds the frame pump's fixed step
// size (hp6::runtime::initialize_frame_pump_timing), assigns the frame
// pump's "this" object (DAT_0089eb0c = &PTR_PTR_0087eadc — a real,
// already-existing game object; FrameClock.h's note that its concrete
// type was unidentified is resolved to the extent that this is where it
// is assigned, though the type behind PTR_PTR_0087eadc itself remains
// unexplored), registers every lifecycle/render message, hashes in the
// debug audio subsystem, shows the (currently dead) debug build overlay
// twice, creates the TimeManager, invokes vtable slot 2 of the engine-
// service object at DAT_00d6af14 (its real construction — found this
// session to be a genuine C++ global static initializer at 0x0070f720,
// running before WinMain, not something FUN_005dcdf0 itself triggers —
// is still NOT reconstructed; this call reuses DeviceCreate.cpp's
// existing no-op placeholder for that global, the same way FUN_005dca10
// already does for its own call on a different vtable slot of the same
// object), and finishes by setting DAT_00cef718/DAT_00cef69f/
// DAT_00cef69e to their fixed post-init values.
void initialize_engine_core_systems();

} // namespace hp6::presentation
