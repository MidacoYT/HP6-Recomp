#pragma once

#include <cstdint>

namespace hp6::presentation {

// FUN_00fdbad0 / thunk 0x00450640: append a bitmap to the manager's
// pointer vector, growing through the shared EASTL allocator if needed.
void register_presentation_bitmap(void* presentation, void* bitmap,
                                  void* (*allocate)(std::uint32_t),
                                  void (*release)(void*));

// FUN_004505f0 (manager vtable slot +0x28): visit the presentation object's
// bitmap list and invoke bitmap vtable slot +0x34 in the active space.
// Both objects use their original x86 layouts.
void dispatch_presentation_bitmaps(void* presentation, void* transform_context,
                                   void* camera_context);

// FUN_00465e10: traverse the presentation group's child vector in its
// context at +0x0c (vector begin/end at context+0x298/+0x29c).
void dispatch_presentation_group(void* group, void* transform_context,
                                 void* camera_context);

} // namespace hp6::presentation
