#include "presentation/BitmapRenderQuad.h"

namespace hp6::presentation {

bool submit_bitmap_render_quad_mode2(const void* bitmap, void* transform_context,
                                     const BitmapCoordinateSpace* spaces) {
    BitmapRenderQuad quad;
    if (!build_bitmap_render_quad_mode2(bitmap, spaces, quad)) {
        return false;
    }
    layout_text_descriptor(transform_context, quad.descriptor);
    return true;
}

} // namespace hp6::presentation
