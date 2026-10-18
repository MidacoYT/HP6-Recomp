#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x005dc750 (cdecl). Allocates `size` bytes through the shared, lazily
// initialized "named allocation" service (hp6::platform::service_holder,
// src/platform/HandleRegistry.h; same lazy-init block, reused rather
// than duplicated), tagged with `name` for memory tracking/debugging —
// the tag is not a lookup key. Used throughout GameServicesCreate
// (0x005d6140, see architecture/startup_path.md) to construct the
// engine's ~20 top-level singletons (FMVDataManager,
// PresentationTaskHandler, GameFlowStateController, ...).
void* allocate_tagged(std::int32_t size, const char* name);

// Not a standalone ReAgent address; factored out of 0x005d2b30 and the
// per-screen `ScreenTask_Screen*_Enqueue` constructors (e.g.
// 0x0050b920), which each repeat this exact sequence inline: fetch
// PresentationTaskHandler's own nested allocator sub-object at
// field+8, and call its vtable slot 0 (thiscall) with `size` and a
// debug tag. Distinct from allocate_tagged above (a different, global
// allocator reached through hp6::platform::service_holder).
void* allocate_via_task_handler(void* task_handler, std::int32_t size, const char* tag);

// 0x005d2b30 (stdcall; `task_handler` is PresentationTaskHandler's own
// "this" — its field+0xc is the linked list's head pointer). Wraps
// `task` in a new queue node and appends it to that list. The node is
// allocated through task_handler's own nested allocator sub-object at
// field+8 (a distinct, per-subsystem allocator — NOT allocate_tagged
// above), tagged "Presentation:TaskHandler::STaskItem". Immediately
// calls the task's own vtable slot 1 ("IsReady"-style query, thiscall,
// no explicit arguments); when that reports false (0), marks the
// task's byte field+4 as 1. Returns `task` unchanged, or null when
// `task` is null (no node is created).
void* submit_task(void* task_handler, void* task);

} // namespace hp6::presentation
