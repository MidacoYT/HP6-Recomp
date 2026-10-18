extern uintptr_t (__cdecl *PTR__beginthreadex_007131d0)(
    void*, uint, _StartAddress*, void*, uint, uint*);

uintptr_t __cdecl _beginthreadex(
    void* _Security,
    uint _StackSize,
    _StartAddress* _StartAddress,
    void* _ArgList,
    uint _InitFlag,
    uint* _ThrdAddr)
{
    return PTR__beginthreadex_007131d0(
        _Security, _StackSize, _StartAddress, _ArgList, _InitFlag, _ThrdAddr);
}
