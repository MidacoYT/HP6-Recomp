void __fastcall FUN_006d928b(int param_1)
{
    // The member name at offset 4 is unresolved in the supplied evidence.
    InitializeCriticalSection(
        reinterpret_cast<LPCRITICAL_SECTION>(param_1 + 4));
}
