// Local behavior checks for the archive header batch (026). They exercise the
// reconstructed interfaces only; they do not compare against the binary.
#include "archive/Header.h"

#include <cstdint>
#include <cstdio>

namespace {

int failures = 0;

void expect(const char* label, bool condition) {
    if (!condition) {
        ++failures;
        std::printf("FAIL %s\n", label);
    }
}

} // namespace

int main() {
    using namespace hp6::archive;

    const std::uint8_t one[]{0xABu};
    expect("read1", read_partial_uint(one, 1) == 0xABu);
    expect("read0", read_partial_uint(one, 0) == 0u);
    expect("read5", read_partial_uint(one, 5) == 0u);

    // Widths 3 and 4 are big-endian reads (evidenced by the "BIGF" literal).
    const std::uint8_t three[]{0x01u, 0x02u, 0x03u};
    expect("read3", read_partial_uint(three, 3) == 0x00010203u);
    const std::uint8_t four[]{0x01u, 0x02u, 0x03u, 0x04u};
    expect("read4", read_partial_uint(four, 4) == 0x01020304u);

    const std::uint8_t packed[]{0x12u, 0x34u, 0x56u, 0x78u, 0x9Au};
    expect("pack5", pack_bytes_be(packed, 5) == 0x123456789Au);
    expect("pack0", pack_bytes_be(packed, 0) == 0u);

    // The exporter "BIGF" file bytes read to 0x42494746 under the
    // big-endian reader.
    const std::uint8_t bigf[]{0x42u, 0x49u, 0x47u, 0x46u,
                              0x00u, 0x00u, 0x00u, 0x00u,
                              0x00u, 0x00u, 0x00u, 0x00u,
                              0x01u, 0x02u, 0x03u, 0x04u,
                              0x00u, 0x00u, 0x00u, 0x00u};
    expect("bigf", classify_container(bigf) == 2u);
    expect("bigf_offset", header_payload_offset(bigf) == 0x01020304u);

    // "BIG?": the 4th byte does not change the classification to 3.
    const std::uint8_t bigq[]{0x42u, 0x49u, 0x47u, 0x10u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    expect("bigq", classify_container(bigq) == 3u);

    // 0x53687000: bytes 'S','h','p',0x00.
    const std::uint8_t sh[]{0x53u, 0x68u, 0x70u, 0x00u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    expect("sh", classify_container(sh) == 2u);

    const std::uint8_t unknown[]{0x00u, 0x00u, 0x00u, 0x00u, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    expect("unknown", classify_container(unknown) == 0u);

    // Type-1 signed short marker 0xC0FB: bytes 0xC0, 0xFB.
    // Payload offset = (p2<<8|p3) + 4.
    std::uint8_t type1[0x30]{};
    type1[0] = 0xC0u;
    type1[1] = 0xFBu;
    type1[2] = 0x00u;
    type1[3] = 0x18u; // offset = 0x18 + 4 = 0x1c
    expect("m1", classify_container(type1) == 1u);
    expect("m1_offset", header_payload_offset(type1) == 0x1Cu);

    // Tag block ends 8 bytes before the payload offset: base = +0x14.
    type1[0x14] = 'A';
    type1[0x15] = '5';
    type1[0x16] = '0';
    type1[0x17] = '3';
    type1[0x18] = 0x01u;
    type1[0x19] = 0x02u;
    type1[0x1a] = 0x03u;
    type1[0x1b] = 0x04u;
    std::int32_t digits = 0;
    std::uint32_t tag = 0;
    expect("tag_size", extract_numeric_tag(type1, &digits, &tag) == 8u);
    expect("tag_digits", digits == 503);
    expect("tag_value", tag == read_partial_uint(type1 + 0x18, 4));

    // Missing tag before the boundary yields 0 and no writes.
    std::uint8_t quiet[0x30]{};
    quiet[0] = 0xFBu; quiet[1] = 0xC0u; quiet[2] = 0x18u;
    std::int32_t no_digits = 1234;
    std::uint32_t no_tag = 5678;
    expect("tag_absent", extract_numeric_tag(quiet, &no_digits, &no_tag) == 0u);
    expect("tag_no_write", no_digits == 1234 && no_tag == 5678u);

    if (failures != 0) {
        std::printf("archive_header_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("archive_header_smoke: all checks passed\n");
    return 0;
}
