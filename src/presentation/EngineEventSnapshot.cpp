#include "presentation/EngineEventSnapshot.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hp6::presentation {
namespace {
#ifdef _MSC_VER
using ResetCall = void (__thiscall*)(void*);
using NextCall = const std::uint8_t* (__thiscall*)(void*);
#else
using ResetCall = void (*)(void*);
using NextCall = const std::uint8_t* (*)(void*);
#endif

void* method_at(void* object, std::uint32_t offset) {
    return *reinterpret_cast<void**>(
        *reinterpret_cast<std::uint8_t**>(object) + offset);
}
} // namespace

void snapshot_engine_events(void* source) {
    auto* bytes = static_cast<std::uint8_t*>(source);
    auto* stream = *reinterpret_cast<void**>(bytes + 0x0c);
    if (stream != nullptr) {
        reinterpret_cast<ResetCall>(method_at(stream, 8))(stream);
    }

    auto* records = *reinterpret_cast<std::uint8_t**>(bytes + 8);
    *reinterpret_cast<std::uint32_t*>(bytes + 0x28) = 0;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x2c) = 0;
    std::memset(records, 0, 0x480);

    while ((stream = *reinterpret_cast<void**>(bytes + 0x0c)) != nullptr) {
        const auto* record = reinterpret_cast<NextCall>(method_at(stream, 0x18))(stream);
        if (record == nullptr) break;
        auto& count = *reinterpret_cast<std::int32_t*>(bytes + 0x2c);
        if (count < 0x20) {
            auto* destination = records + static_cast<std::size_t>(count) * 0x24;
            ++count;
            // The first dword stays zero from memset. The x86 routine copies
            // eight dwords at source/destination offsets +4..+0x20.
            std::memcpy(destination + 4, record + 4, 0x20);
        }
    }
}

} // namespace hp6::presentation
