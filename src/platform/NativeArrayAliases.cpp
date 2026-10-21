// These Ghidra labels are interior addresses in the original PE .data.
// The native render objects are compiled to load these relocation pointers.
// The test objects retain their separate fixture arrays.
#include <cstdint>

extern std::int32_t DAT_00b108a0[257];
extern std::int32_t DAT_009968d8[257];
extern std::int32_t DAT_00c75188[236];
extern std::uint8_t DAT_00f38b10[0x600 * 0xc4];

std::uint8_t* DAT_00b108a4 = reinterpret_cast<std::uint8_t*>(DAT_00b108a0) + 4;
std::uint8_t* DAT_009968dc = reinterpret_cast<std::uint8_t*>(DAT_009968d8) + 4;
std::uint8_t* DAT_00c751a4 = reinterpret_cast<std::uint8_t*>(DAT_00c75188) + 0x1c;
std::uint8_t* DAT_00f38b28 = DAT_00f38b10 + 0x18;
