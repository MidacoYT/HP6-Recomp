#include <cassert>
#include <cstdint>
#include <cstring>

extern void overlay_rebuild_order_005ef430();
extern std::uint8_t* DAT_0089f690;
extern std::uint32_t DAT_0089f3c0[0x90];
extern std::uint8_t DAT_0089f600[0x90];
extern std::uint8_t DAT_009984f0[0x90];
extern std::uint32_t DAT_0085b53c;
extern std::uint32_t DAT_008a01d8;
extern std::uint32_t DAT_008a01dc;

namespace {
template <typename T> void put(std::uint8_t* at, unsigned offset, T value) {
    std::memcpy(at + offset, &value, sizeof value);
}
} // namespace

int main() {
    static_assert(sizeof(void*) == 4, "The original descriptor layout is x86");
    alignas(4) std::uint8_t node1[0x48]{};
    alignas(4) std::uint8_t node2[0x48]{};
    alignas(4) std::uint8_t phase0[0x14]{};
    alignas(4) std::uint8_t phase1a[0x14]{};
    alignas(4) std::uint8_t phase1b[0x14]{};
    alignas(4) std::uint8_t phase2[0x14]{};

    put(node1, 0x30, phase0);
    put(node1, 0x34, phase1a);
    put(node1, 0x38, phase2);
    put(node1, 0x3c, node2);
    put(node2, 0x34, phase1b);
    put(phase0, 4, std::uint32_t{0x1234});
    put(phase0, 0x0a, std::uint16_t{3});
    put(phase1a, 4, std::uint32_t{0x5678});
    put(phase1a, 8, std::int16_t{9});
    put(phase1a, 0x0a, std::uint16_t{4});
    put(phase1b, 4, std::uint32_t{0x9abc});
    put(phase1b, 8, std::int16_t{2});
    put(phase1b, 0x0a, std::uint16_t{5});
    put(phase2, 4, std::uint32_t{0xdef0});
    put(phase2, 0x0a, std::uint16_t{6});
    DAT_0089f690 = node1;
    DAT_008a01d8 = 71;
    overlay_rebuild_order_005ef430();
    assert(DAT_0085b53c == 4);
    assert(DAT_008a01dc == 71);
    assert(DAT_0089f600[0] == 3 && DAT_0089f600[1] == 5);
    assert(DAT_0089f600[2] == 4 && DAT_0089f600[3] == 6);
    assert(std::memcmp(DAT_009984f0, DAT_0089f600, 4) == 0);
    assert(DAT_0089f3c0[3] == 0x1234 && DAT_0089f3c0[5] == 0x9abc);
}
