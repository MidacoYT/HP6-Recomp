#include <cstdint>
#include <cstring>

// 0x005EF430 in the SHA-verified unprotected x86 image. These objects occupy
// .data in both images; their initial bytes were checked in the protected PE.
std::uint32_t DAT_0089f3c0[0x90] = {};
std::uint8_t DAT_0089f600[0x90] = {};
std::uint8_t* DAT_0089f690 = nullptr;
std::uint32_t DAT_008a01d8 = 0;
std::uint32_t DAT_008a01dc = 0;
std::uint8_t DAT_009984f0[0x90] = {};
std::uint32_t DAT_0085b53c = 0x90;

namespace {
template <typename T> T& field(std::uint8_t* address, std::uint32_t offset) {
    return *reinterpret_cast<T*>(address + offset);
}

void append_descriptor(std::uint8_t* descriptor, std::uint32_t& count) {
    const auto id = field<std::uint16_t>(descriptor, 0x0a);
    DAT_0089f600[count] = static_cast<std::uint8_t>(id);
    DAT_0089f3c0[id] = field<std::uint32_t>(descriptor, 4);
    field<std::uint32_t>(descriptor, 0x10) = count++;
}
} // namespace

void overlay_rebuild_order_005ef430() {
    std::memset(DAT_0089f3c0, 0, sizeof DAT_0089f3c0);
    for (std::uint32_t index = 0; index < 0x90; ++index) {
        DAT_0089f600[index] = static_cast<std::uint8_t>(index);
    }

    std::uint32_t count = 0;
    for (std::uint32_t phase = 0; phase < 3; ++phase) {
        if (phase != 1) {
            for (auto* node = DAT_0089f690; node != nullptr;
                 node = field<std::uint8_t*>(node, 0x3c)) {
                for (auto* descriptor = field<std::uint8_t*>(node, 0x30 + phase * 4);
                     descriptor != nullptr;
                     descriptor = field<std::uint8_t*>(descriptor, 0)) {
                    append_descriptor(descriptor, count);
                }
            }
            continue;
        }

        std::uint8_t* first = nullptr;
        std::uint8_t* last = nullptr;
        for (auto* node = DAT_0089f690; node != nullptr;
             node = field<std::uint8_t*>(node, 0x3c)) {
            field<std::uint8_t*>(node, 0x40) = field<std::uint8_t*>(node, 0x34);
            field<std::uint8_t*>(node, 0x44) = nullptr;
            if (field<std::uint8_t*>(node, 0x40) != nullptr) {
                if (last != nullptr) field<std::uint8_t*>(last, 0x44) = node;
                else first = node;
                last = node;
            }
        }
        while (first != nullptr) {
            std::uint8_t* selected = nullptr;
            std::uint8_t* previous = nullptr;
            std::uint8_t* selected_previous = nullptr;
            for (auto* node = first; node != nullptr;
                 previous = node, node = field<std::uint8_t*>(node, 0x44)) {
                const auto* descriptor = field<std::uint8_t*>(node, 0x40);
                if (selected == nullptr ||
                    field<std::int16_t>(const_cast<std::uint8_t*>(descriptor), 8) <
                        field<std::int16_t>(field<std::uint8_t*>(selected, 0x40), 8)) {
                    selected = node;
                    selected_previous = previous;
                }
            }
            auto* descriptor = field<std::uint8_t*>(selected, 0x40);
            append_descriptor(descriptor, count);
            field<std::uint8_t*>(selected, 0x40) = field<std::uint8_t*>(descriptor, 0);
            if (field<std::uint8_t*>(selected, 0x40) == nullptr) {
                if (selected_previous == nullptr) first = field<std::uint8_t*>(selected, 0x44);
                else field<std::uint8_t*>(selected_previous, 0x44) =
                    field<std::uint8_t*>(selected, 0x44);
            }
        }
    }
    DAT_008a01dc = DAT_008a01d8;
    std::memcpy(DAT_009984f0, DAT_0089f600, count);
    DAT_0085b53c = count;
}
