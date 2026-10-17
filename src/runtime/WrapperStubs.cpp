#include "runtime/WrapperStubs.h"

namespace hp6::runtime {

// Every body writes nothing and returns nothing; the exported x86 shows
// bare RET (n) epilogues. The word arguments exist only to mirror the
// stdcall stack cleaning of the binary.

void wrap_noop_006b54bb(std::uint8_t* object, std::uint32_t arg) {
    (void)object;
    (void)arg;
}

void wrap_noop_006b54be(std::uint8_t* object) {
    (void)object;
}

void wrap_noop_006b454a(std::uint8_t* wrapper, std::uint32_t arg) {
    (void)wrapper;
    (void)arg;
}

void wrap_noop_006b4555(std::uint8_t* wrapper, std::uint32_t arg) {
    (void)wrapper;
    (void)arg;
}

void wrap_noop_006b4560(std::uint8_t* wrapper, std::uint32_t arg_1,
                        std::uint32_t arg_2) {
    (void)wrapper;
    (void)arg_1;
    (void)arg_2;
}

bool wrap_true_006b54b8() {
    return true;
}

void reset_wrapper_block_006b45ae(std::uint8_t* block) {
    // Exported x86: OR [ECX],-1; OR [ECX+4],-1; XOR EAX,EAX; MOV
    // [ECX+8..+0x10],EAX; MOV byte [ECX+0x14],AL; MOV byte [ECX+0x15],AL.
    *reinterpret_cast<std::uint32_t*>(block) = 0xffffffffu;
    *reinterpret_cast<std::uint32_t*>(block + 4) = 0xffffffffu;
    *reinterpret_cast<std::uint32_t*>(block + 8) = 0;
    *reinterpret_cast<std::uint32_t*>(block + 0xc) = 0;
    *reinterpret_cast<std::uint32_t*>(block + 0x10) = 0;
    block[0x14] = 0;
    block[0x15] = 0;
}

} // namespace hp6::runtime
