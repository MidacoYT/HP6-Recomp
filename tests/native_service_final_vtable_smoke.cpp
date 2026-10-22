#include <cstdint>

extern void* PTR_LAB_008201f8[4];
extern "C" int __cdecl _purecall();

namespace {
void* released = nullptr;
}

namespace hp6::platform {
void release_through_service(void* object) { released = object; }
}

int main() {
    static_assert(sizeof(void*) == 4);
    if (PTR_LAB_008201f8[1] != reinterpret_cast<void*>(&_purecall) ||
        PTR_LAB_008201f8[2] != reinterpret_cast<void*>(&_purecall) ||
        PTR_LAB_008201f8[3] != reinterpret_cast<void*>(&_purecall)) return 1;

    void* object[] = {nullptr};
    using Destroy = void* (__thiscall*)(void*, std::uint32_t);
    auto destroy = reinterpret_cast<Destroy>(PTR_LAB_008201f8[0]);
    if (destroy(object, 0) != object || object[0] != PTR_LAB_008201f8 ||
        released != nullptr) return 2;
    if (destroy(object, 1) != object || released != object) return 3;
    return 0;
}
