#include <cassert>
#include <cstdint>
#include <cstring>

extern std::uint8_t* DAT_00fcade8;
extern void* __stdcall screen_find_by_name_0053cc00(std::uint8_t*, const char*);
extern std::int32_t command_screen_active_0048ace0(const char*);

namespace {
struct Screen {
    void* vtable;
    const char* name;
    const char* get_name() { return name; }
};

template <typename T> void put(std::uint8_t* address, unsigned offset, T value) {
    std::memcpy(address + offset, &value, sizeof value);
}
} // namespace

int main() {
    static_assert(sizeof(void*) == 4, "Original list records use x86 pointers");
    alignas(4) std::uint8_t manager[0x4530]{};
    alignas(4) std::uint8_t vtable[0x38]{};
    Screen screen{};
    alignas(4) std::uint8_t known[12]{};
    alignas(4) std::uint8_t active[12]{};
    auto method = &Screen::get_name;
    static_assert(sizeof method == sizeof(void*));
    put(vtable, 0x34, method);
    screen.vtable = vtable;
    screen.name = "ScreenLanguageSelect";
    auto* all_head = manager + 0x4518;
    auto* active_head = manager + 0x4524;
    put(all_head, 0, known);
    put(known, 0, all_head);
    put(known, 8, &screen);
    put(active_head, 0, active_head);
    DAT_00fcade8 = manager;

    assert(screen_find_by_name_0053cc00(manager, "ScreenLanguageSelect") == &screen);
    assert(screen_find_by_name_0053cc00(manager, "ScreenMainMenu") == nullptr);
    assert(command_screen_active_0048ace0("ScreenLanguageSelect") == 0);
    put(active_head, 0, active);
    put(active, 0, active_head);
    put(active, 8, &screen);
    assert(command_screen_active_0048ace0("ScreenLanguageSelect") == 1);
    assert(command_screen_active_0048ace0("ScreenMainMenu") == 0);
}
