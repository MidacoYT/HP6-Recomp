#include "presentation/PresentationRenderDispatch.h"

#include <cstdint>
#include <cstring>


namespace hp6::presentation {
namespace {
template <typename T> T field(const void* object, std::size_t offset) {
    T value{};
    std::memcpy(&value, static_cast<const std::uint8_t*>(object) + offset,
                sizeof(value));
    return value;
}
template <typename T> void set_field(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(object) + offset, &value, sizeof(value));
}

using RenderSlot = void (__thiscall *)(void*, void*, void*);
std::uint32_t pointer_count(void** begin, void** end) {
    // The original subtracts x86 addresses. Integer subtraction also covers
    // its empty, null-initialized vectors without C++ null-pointer arithmetic.
    return static_cast<std::uint32_t>(
        (reinterpret_cast<std::uintptr_t>(end) -
         reinterpret_cast<std::uintptr_t>(begin)) / sizeof(void*));
}
} // namespace

void register_presentation_bitmap(void* presentation, void* bitmap,
                                  void* (*allocate)(std::uint32_t),
                                  void (*release)(void*)) {
    auto* begin = field<void**>(presentation, 0x4e8);
    auto* end = field<void**>(presentation, 0x4ec);
    auto* capacity = field<void**>(presentation, 0x4f0);
    const auto count = pointer_count(begin, end);
    if (count == 0x21f) {
        return;
    }
    if (end == capacity) {
        const auto next_capacity = count == 0 ? 1u : count * 2u;
        auto* grown = static_cast<void**>(allocate(next_capacity * 4));
        if (count != 0) {
            std::memcpy(grown, begin, count * 4);
        }
        // 00540ee0 compares the old begin with the inline-storage pointer
        // held at manager+0x4f8 before releasing it.
        if (begin != nullptr && begin != field<void*>(presentation, 0x4f8)) {
            release(begin);
        }
        begin = grown;
        end = begin + count;
        capacity = begin + next_capacity;
        set_field(presentation, 0x4e8, begin);
        set_field(presentation, 0x4f0, capacity);
    }
    *end++ = bitmap;
    set_field(presentation, 0x4ec, end);
    const auto high_water = field<std::uint32_t>(presentation, 0xd7c);
    if (high_water < count + 1) {
        set_field(presentation, 0xd7c, count + 1);
    }
}

void dispatch_presentation_bitmaps(void* presentation, void* transform_context,
                                   void* camera_context) {
    // 004505f0 walks [this+0x4e8, this+0x4ec) as four-byte pointers.
    auto* current = field<void**>(presentation, 0x4e8);
    auto* end = field<void**>(presentation, 0x4ec);
    const auto active_space = field<std::uint32_t>(transform_context, 0x3a8);
    while (current != end) {
        void* bitmap = *current++;
        if (field<std::uint16_t>(bitmap, 8) == active_space) {
            const auto* vtable = field<const std::uint8_t*>(bitmap, 0);
            const auto render = field<RenderSlot>(vtable, 0x34);
            render(bitmap, transform_context, camera_context);
        }
    }
}

void dispatch_presentation_group(void* group, void* transform_context,
                                 void* camera_context) {
    std::uint32_t index = 0;
    for (;;) {
        const auto* context = field<const std::uint8_t*>(group, 0x0c);
        auto* begin = field<void**>(context, 0x298);
        auto* end = field<void**>(context, 0x29c);
        if (index >= pointer_count(begin, end)) {
            return;
        }
        void* child = begin[index++];
        const auto* vtable = field<const std::uint8_t*>(child, 0);
        const auto render = field<RenderSlot>(vtable, 0x28);
        render(child, transform_context, camera_context);
    }
}

} // namespace hp6::presentation
