// The 26 dwords at 0x00715DA8 in the unprotected HP6 x86 PE all point
// to the CRT _purecall thunk at 0x006B92B0.
// SHA-256: 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10

extern "C" int __cdecl _purecall();

#define HP6_PURE reinterpret_cast<void*>(&_purecall)
void* PTR_LAB_00715da8[26] = {
    HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE,
    HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE,
    HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE,
    HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE,
    HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE, HP6_PURE,
    HP6_PURE,
};
#undef HP6_PURE
