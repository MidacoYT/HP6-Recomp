#include "presentation/PresentationRenderDispatch.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cstdlib>

template <typename T> void put(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(object) + offset, &value, sizeof(value));
}

static void* seen[3]{};
static int calls = 0;
void* allocate(std::uint32_t bytes) { return std::malloc(bytes); }
void release(void* block) { std::free(block); }
struct Renderer {
    void render(void* transform, void* camera) {
        seen[0] = this;
        seen[1] = transform;
        seen[2] = camera;
        ++calls;
    }
};
struct Manager {
    void render(void* transform, void* camera) {
        hp6::presentation::dispatch_presentation_bitmaps(this, transform, camera);
    }
};

int main() {
    static_assert(sizeof(void*) == 4, "the original presentation list is x86");
    std::uint8_t presentation[0xd80]{};
    std::uint8_t transform[0x3ac]{};
    std::uint8_t camera[4]{};
    std::uint8_t matching[12]{};
    std::uint8_t other[12]{};
    std::uint8_t vtable[0x38]{};
    static_assert(sizeof(&Renderer::render) == sizeof(void*));
    put(vtable, 0x34, &Renderer::render);
    put(matching, 0, vtable);
    put(other, 0, vtable);
    put(matching, 8, std::uint16_t{7});
    put(other, 8, std::uint16_t{8});
    put(transform, 0x3a8, std::uint32_t{7});
    void* entries[3]{};
    put(presentation, 0x4e8, entries);
    put(presentation, 0x4ec, entries);
    put(presentation, 0x4f0, entries + 3);
    put(presentation, 0x4f8, entries);
    hp6::presentation::register_presentation_bitmap(presentation, other, allocate, release);
    hp6::presentation::register_presentation_bitmap(presentation, matching, allocate, release);
    hp6::presentation::register_presentation_bitmap(presentation, other, allocate, release);
    assert(entries[0] == other && entries[1] == matching &&
           entries[2] == other);
    std::uint32_t high_water{};
    std::memcpy(&high_water, presentation + 0xd7c, sizeof(high_water));
    assert(high_water == 3);

    hp6::presentation::dispatch_presentation_bitmaps(presentation, transform, camera);
    assert(calls == 1 && seen[0] == matching && seen[1] == transform &&
           seen[2] == camera);
    put(transform, 0x3a8, std::uint32_t{0x10007});
    hp6::presentation::dispatch_presentation_bitmaps(presentation, transform, camera);
    assert(calls == 1); // The source object's space is only 16 bits.

    hp6::presentation::register_presentation_bitmap(presentation, matching, allocate, release);
    std::memcpy(&high_water, presentation + 0xd7c, sizeof(high_water));
    assert(high_water == 4); // The fourth append grows the three-slot buffer.
    put(transform, 0x3a8, std::uint32_t{7});
    hp6::presentation::dispatch_presentation_bitmaps(presentation, transform, camera);
    assert(calls == 3);

    std::uint8_t group[0x10]{};
    std::uint8_t context[0x2a0]{};
    std::uint8_t manager_vtable[0x2c]{};
    static_assert(sizeof(&Manager::render) == sizeof(void*));
    put(manager_vtable, 0x28, &Manager::render);
    put(presentation, 0, manager_vtable);
    void* children[] = {presentation};
    put(group, 0x0c, context);
    put(context, 0x298, children);
    put(context, 0x29c, children + 1);
    hp6::presentation::dispatch_presentation_group(group, transform, camera);
    assert(calls == 5 && seen[0] == matching);
    put(context, 0x298, static_cast<void**>(nullptr));
    put(context, 0x29c, static_cast<void**>(nullptr));
    hp6::presentation::dispatch_presentation_group(group, transform, camera);
    assert(calls == 5);

    void** grown{};
    std::memcpy(&grown, presentation + 0x4e8, sizeof(grown));
    release(grown);
}
