#include <array>
#include <cassert>
#include <cstdint>
#include <cstring>

void* DAT_00cef710 = nullptr;
extern std::uint32_t DAT_00cef6a4;
extern std::uint32_t DAT_00cef6a8;
extern const char* service_context_registry_name_005db380(const char*);

int main() {
    std::array<std::uint8_t, 0x1f808> memory{};
    DAT_00cef710 = memory.data();
    *reinterpret_cast<char**>(memory.data() + 0x1f804) =
        reinterpret_cast<char*>(memory.data() + 0x5804);

    const char* first = service_context_registry_name_005db380("Menu::Language");
    assert(first == reinterpret_cast<char*>(memory.data() + 0x5804));
    assert(std::strcmp(first, "Menu::Language") == 0);
    assert(service_context_registry_name_005db380("Menu::Language") == first);
    const char* different_case =
        service_context_registry_name_005db380("menu::language");
    assert(different_case != first);
    assert(*reinterpret_cast<std::uint32_t*>(memory.data() + 0x400) == 2);
    assert(DAT_00cef6a4 == 2);
    assert(DAT_00cef6a8 == 30);

    *reinterpret_cast<char**>(memory.data() + 0x1f804) =
        reinterpret_cast<char*>(memory.data() + 0x1f803);
    assert(std::strcmp(service_context_registry_name_005db380("too long"),
                       "ContextRegistry-StringOverflow") == 0);
    *reinterpret_cast<char**>(memory.data() + 0x1f804) =
        reinterpret_cast<char*>(memory.data() + 0x5804);
    *reinterpret_cast<std::uint32_t*>(memory.data() + 0x400) = 0xa80;
    assert(std::strcmp(service_context_registry_name_005db380("new name"),
                       "ContextRegistry-DictionaryOverflow") == 0);
}
