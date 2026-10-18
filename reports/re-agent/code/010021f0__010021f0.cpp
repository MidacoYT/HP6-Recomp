void __stdcall FUN_010021f0()
{
    // The input is passed in ESI; its structure and member names are unresolved.
    std::int32_t* values;
    __asm mov values, esi

    FUN_0064c300(0x13, values[1]);
    FUN_0064c300(0x14, values[2]);
    FUN_0064c300(0x1b, values[3]);
    FUN_0064c300(0x80, values[4]);
    FUN_0064c300(0x81, values[5]);
    FUN_0064c300(0x82, values[6]);
    FUN_0064c300(0x83, values[7]);
    FUN_0064c300(0x84, values[8]);
    FUN_0064c300(0x85, values[9]);
    FUN_0064c300(0x86, values[10]);
    FUN_0064c300(0x87, values[11]);
    FUN_0064c300(0xab, values[12]);

    FUN_0064c3b0(0, 1, values[13]);
    FUN_0064c3b0(0, 2, values[14]);
    FUN_0064c3b0(0, 3, values[15]);
    FUN_0064c3b0(0, 4, values[16]);
    FUN_0064c3b0(0, 5, values[17]);
    FUN_0064c3b0(0, 6, values[18]);

    _DAT_008813a4 = values[13];
    _DAT_008813a8 = values[14];
    _DAT_008813ac = values[15];
    _DAT_008813b0 = values[16];
    _DAT_008813b4 = values[17];
    _DAT_008813b8 = values[18];

    _DAT_00881374 = values[1];
    _DAT_00881378 = values[2];
    _DAT_0088137c = values[3];
    _DAT_00881380 = values[4];
    _DAT_00881384 = values[5];
    _DAT_00881388 = values[6];
    _DAT_0088138c = values[7];
    _DAT_00881390 = values[8];
    _DAT_00881394 = values[9];
    _DAT_00881398 = values[10];
    _DAT_0088139c = values[11];
    _DAT_008813a0 = values[12];

    DAT_0086527c = values[0];
}