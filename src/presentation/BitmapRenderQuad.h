#pragma once

#include <cstdint>
#include <string>

#include "presentation/TextLayout.h"

namespace hp6::presentation {

// The producer-owned portion of PluginPresentationBitmap_BuildRenderQuad
// (0x0056baa0). The caller supplies the already-selected camera matrix and
// the three entries of the coordinate-space table at 0x00892000. This keeps
// the two indirect engine lookups outside the byte layout reconstructed here.
struct BitmapRenderQuad {
    float matrix[16]{};
    RenderTextDescriptor descriptor{};
    std::wstring fallback_text;
};

// 12-byte entries at 0x00892000; this+0x34 indexes this table. The
// original stores the three dwords at descriptor +0x44..+0x50.
struct BitmapCoordinateSpace {
    const char* name;
    std::uint32_t match_word;
    std::uint32_t match_dword;
};
#if INTPTR_MAX == INT32_MAX
static_assert(sizeof(BitmapCoordinateSpace) == 0xc);
#endif

// `bitmap` is the original x86 object layout. `out` owns the matrix and any
// fallback text referenced by its descriptor; do not copy/move it afterwards.
// Returns false when the source's +0x58 visibility byte is clear.
bool build_bitmap_render_quad(const void* bitmap, const float* camera_matrix,
                              const char* match_name, std::uint32_t match_word,
                              std::uint32_t match_dword, BitmapRenderQuad& out);

// The directly traced `DAT_00fcae50+8 == 2` branch of 0x0056baa0:
// bitmap+4 points to the parent whose 16-float camera matrix starts at
// +0x10. The other branch goes through 0x006236d0 and remains separate.
bool build_bitmap_render_quad_mode2(const void* bitmap,
                                    const BitmapCoordinateSpace* spaces,
                                    BitmapRenderQuad& out);

// 0x0056baa0's final call to 0x005e92d0 on that same mode-2 branch.
// `transform_context` is the original param_1, whose +0x40 matrix is
// consumed by submit_text_runs.
bool submit_bitmap_render_quad_mode2(const void* bitmap, void* transform_context,
                                     const BitmapCoordinateSpace* spaces);

} // namespace hp6::presentation
