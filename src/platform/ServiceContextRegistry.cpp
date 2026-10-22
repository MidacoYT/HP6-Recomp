#include <cstddef>
#include <cstdint>
#include <cstring>

extern void* DAT_00cef710;

// Zero-initialized counters at 0x00CEF6A4 and 0x00CEF6A8 in both PEs.
std::uint32_t DAT_00cef6a4 = 0;
std::uint32_t DAT_00cef6a8 = 0;

namespace {
struct Entry {
    Entry* next;
    const char* name;
};

struct Registry {
    Entry* buckets[0x100];
    std::uint32_t count;
    Entry entries[0xa80];
    // The entries end at +0x5804. The string pool ends at +0x1f804.
    char strings[0x1a000];
    char* next_string;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(Registry, count) == 0x400);
static_assert(offsetof(Registry, entries) == 0x404);
static_assert(offsetof(Registry, strings) == 0x5804);
static_assert(offsetof(Registry, next_string) == 0x1f804);
#endif

std::uint8_t bucket_for(const char* name) {
    // 0x00404620: case-folded 0x1003F multiplicative hash, low byte.
    std::uint32_t hash = 0;
    if (name != nullptr) {
        for (const auto* cursor = reinterpret_cast<const std::uint8_t*>(name);
             *cursor != 0; ++cursor) {
            std::uint32_t letter = *cursor;
            if (letter >= 'A' && letter <= 'Z') letter += 0x20;
            hash = hash * 0x1003fu + letter;
        }
    }
    return static_cast<std::uint8_t>(hash);
}
} // namespace

const char* service_context_registry_name_005db380(const char* name) {
    // 0x005DB380 -> 0x00FF8820. The original caller checks DAT_00CEF710.
    auto* registry = static_cast<Registry*>(DAT_00cef710);
    if (registry->count != 0) {
        for (Entry* entry = registry->buckets[bucket_for(name)]; entry != nullptr;
             entry = entry->next) {
            // The hash folds ASCII case; the comparison at 0x0100ED7E is
            // byte-exact, so differently-cased names remain distinct.
            if (std::strcmp(name, entry->name) == 0) return entry->name;
        }
    }

    const auto length = std::strlen(name) + 1;
    ++DAT_00cef6a4;
    DAT_00cef6a8 += static_cast<std::uint32_t>(length);
    char* const saved = registry->next_string;
    if (saved + length > registry->strings + sizeof(registry->strings))
        return "ContextRegistry-StringOverflow";
    std::memcpy(saved, name, length);

    // 0x005DFF60 -> 0x0100ED00: insert at the bucket head.
    if (registry->count >= 0xa80)
        return "ContextRegistry-DictionaryOverflow";
    const auto bucket = bucket_for(name);
    Entry* const entry = &registry->entries[registry->count++];
    entry->name = saved;
    entry->next = registry->buckets[bucket];
    registry->buckets[bucket] = entry;
    registry->next_string = saved + length;
    return saved;
}
