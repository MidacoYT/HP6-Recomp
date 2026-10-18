#include <cstdint>

// The input is supplied in ESI. Its field names and the meanings of these
// globals are unresolved, so their address-based names are retained.
extern std::uint32_t DAT_0086527c;
extern std::uint32_t _DAT_00881374, _DAT_00881378, _DAT_0088137c;
extern std::uint32_t _DAT_00881380, _DAT_00881384, _DAT_00881388;
extern std::uint32_t _DAT_0088138c, _DAT_00881390, _DAT_00881394;
extern std::uint32_t _DAT_00881398, _DAT_0088139c, _DAT_008813a0;
extern std::uint32_t _DAT_008813a4, _DAT_008813a8, _DAT_008813ac;
extern std::uint32_t _DAT_008813b0, _DAT_008813b4, _DAT_008813b8;

std::uint32_t __cdecl FUN_0064c300(int, int);
std::uint32_t __cdecl FUN_0064c3b0(int, int, int);

// MSVC x86: preserves the original register-supplied input and stdcall ABI.
void __stdcall thunk_FUN_010021f0()
{
    const std::uint32_t* source;
    __asm mov source, esi

    FUN_0064c300(0x13, source[1]);
    FUN_0064c300(0x14, source[2]);
    FUN_0064c300(0x1b, source[3]);
    FUN_0064c300(0x80, source[4]);
    FUN_0064c300(0x81, source[5]);
    FUN_0064c300(0x82, source[6]);
    FUN_0064c300(0x83, source[7]);
    FUN_0064c300(0x84, source[8]);
    FUN_0064c300(0x85, source[9]);
    FUN_0064c300(0x86, source[10]);
    FUN_0064c300(0x87, source[11]);
    FUN_0064c300(0xab, source[12]);

    FUN_0064c3b0(0, 1, source[13]);
    FUN_0064c3b0(0, 2, source[14]);
    FUN_0064c3b0(0, 3, source[15]);
    FUN_0064c3b0(0, 4, source[16]);
    FUN_0064c3b0(0, 5, source[17]);
    FUN_0064c3b0(0, 6, source[18]);

    _DAT_008813a4 = source[13];
    _DAT_008813a8 = source[14];
    _DAT_008813ac = source[15];
    _DAT_008813b0 = source[16];
    _DAT_008813b4 = source[17];
    _DAT_008813b8 = source[18];

    _DAT_00881374 = source[1];
    _DAT_00881378 = source[2];
    _DAT_0088137c = source[3];
    _DAT_00881380 = source[4];
    _DAT_00881384 = source[5];
    _DAT_00881388 = source[6];
    _DAT_0088138c = source[7];
    _DAT_00881390 = source[8];
    _DAT_00881394 = source[9];
    _DAT_00881398 = source[10];
    _DAT_0088139c = source[11];
    _DAT_008813a0 = source[12];

    DAT_0086527c = source[0];
}