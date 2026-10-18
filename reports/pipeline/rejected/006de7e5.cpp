extern void* PTR_FUN_0071b670;
extern void __cdecl FUN_006dcbc7(void* param_1, unsigned int param_2);

void* __thiscall FUN_006de7e5(void* self, unsigned char param_1)
{
    *static_cast<void**>(self) = &PTR_FUN_0071b670;

    if ((param_1 & 1) != 0) {
        FUN_006dcbc7(self, 4);
    }

    return self;
}
