#include <cstdint>

static_assert(sizeof(void*) == 4);

struct Entry_00a68820 {
    std::uint32_t field_0;
    std::uint32_t field_4;
};
static_assert(sizeof(Entry_00a68820) == 8);

extern Entry_00a68820 DAT_00a68820[];
extern std::uint32_t DAT_00cf1a30;
extern void* DAT_00cf1944;

void __stdcall FUN_006525d0()
{
    for (std::uint32_t i = 0; i < DAT_00cf1a30; ++i) {
        void* object = reinterpret_cast<void*>(DAT_00a68820[i].field_4);
        auto vtable = *reinterpret_cast<std::uintptr_t**>(object);
        auto call = reinterpret_cast<void (__thiscall *)(void*)>(vtable[2]);
        call(object);
    }

    for (std::uint32_t i = 0; i < DAT_00cf1a30; ++i) {
        void* object = DAT_00cf1944;
        auto vtable = *reinterpret_cast<std::uintptr_t**>(object);
        auto call = reinterpret_cast<void (__thiscall *)(void*, std::uint32_t, std::uint32_t*)>(
            vtable[0x16c / 4]);
        call(object, DAT_00a68820[i].field_0, &DAT_00a68820[i].field_4);
    }
}
