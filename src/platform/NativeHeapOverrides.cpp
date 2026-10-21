// Generated from verified original PE bytes by tools/audit_native_globals.py.
// SHA-256: 4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954
#include <cstdint>
#include <cstddef>

struct HeapWordOverride { std::uint32_t index, value; };
struct HeapNameOverride { std::uint32_t index; const char* name; std::uint32_t reserved; };
static_assert(sizeof(HeapWordOverride) == 8);
static_assert(sizeof(HeapNameOverride) == 12, "original x86 layout");
HeapWordOverride native_heap_size_overrides[2] = {
    {1u, 0x00C00000u},
    {0u, 0x00A00000u},
};
HeapWordOverride native_heap_flag_overrides[2] = {
    {1u, 0x00000007u},
    {0u, 0x00000004u},
};
HeapNameOverride native_heap_name_overrides[2] = {
    {0u, "RenderHeap", 0u},
    {1u, "SceneHeap", 0u},
};
std::uint8_t* PTR_DAT_008796b4 = reinterpret_cast<std::uint8_t*>(native_heap_size_overrides);
std::uint8_t* PTR_DAT_008796bc = reinterpret_cast<std::uint8_t*>(native_heap_flag_overrides);
std::uint8_t* PTR_DAT_008796cc = reinterpret_cast<std::uint8_t*>(native_heap_name_overrides);
