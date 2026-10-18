#pragma once
#include <cstdint>

namespace hp6::render {

// 0x0064a810. Called from the device-lost handler
// (hp6::render documentation: architecture/startup_path.md) when the
// device is valid. Recreates dynamic resources, then (re)creates the
// depth-stencil surface and, unless disabled, an offscreen render-target
// texture matching the back buffer's format, before binding both and
// refilling the default render state profile. See
// src/render/RenderTargetSetup.cpp for the full vtable cross-reference.
// Kept in its own translation unit (separate from DeviceReset.cpp) so
// that reconstructions which don't need this function's dependency on
// hp6::render::initialize_default_render_state (RenderStateCache.obj,
// with its own large set of default-profile globals) don't have to
// provide definitions for them.
void prepare_render_target_texture();

} // namespace hp6::render
