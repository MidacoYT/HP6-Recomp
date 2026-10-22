#include "presentation/EngineEventSnapshot.h"
#include "presentation/EngineRenderGate.h"

#include <cstdint>
#include <cstring>

void* DAT_00fcaf34 = nullptr;
int dispatch_calls = 0;
void dispatch_engine_tick_005dcb30() { ++dispatch_calls; }

namespace {
struct FakeStream {
    void** vtable;
    int reset_calls = 0;
    int next_calls = 0;
    std::uint8_t items[34][0x24] = {};

    void reset() { ++reset_calls; next_calls = 0; }
    const std::uint8_t* next() {
        const int index = next_calls++;
        return index < 34 ? items[index] : nullptr;
    }
};

template <typename Method>
void* method_address(Method value) {
    union { Method method; void* address; } conversion{};
    conversion.method = value;
    return conversion.address;
}
} // namespace

int main() {
    if (sizeof(void*) != 4) return 0;
    void* vtable[7] = {};
    vtable[2] = method_address(&FakeStream::reset);
    vtable[6] = method_address(&FakeStream::next);
    FakeStream stream{};
    stream.vtable = vtable;
    for (int index = 0; index < 34; ++index) {
        for (int byte = 0; byte < 0x24; ++byte) {
            stream.items[index][byte] = static_cast<std::uint8_t>(index + byte + 1);
        }
    }
    std::uint8_t output[0x480];
    std::memset(output, 0xa5, sizeof output);
    alignas(void*) std::uint8_t source[0x30] = {};
    auto* output_pointer = output;
    std::memcpy(source + 8, &output_pointer, 4);
    auto* stream_pointer = &stream;
    std::memcpy(source + 0x0c, &stream_pointer, 4);
    DAT_00fcaf34 = source;

    hp6::presentation::EngineRenderGateObject gate{};
    gate.simulate(nullptr);
    if (dispatch_calls != 1 || stream.reset_calls != 1 || stream.next_calls != 35) return 1;
    std::uint32_t count = 0;
    std::memcpy(&count, source + 0x2c, 4);
    if (count != 32) return 2;
    for (int index = 0; index < 32; ++index) {
        if (output[index * 0x24] != 0 ||
            std::memcmp(output + index * 0x24 + 4, stream.items[index] + 4, 0x20) != 0)
            return 3;
    }
    return 0;
}
