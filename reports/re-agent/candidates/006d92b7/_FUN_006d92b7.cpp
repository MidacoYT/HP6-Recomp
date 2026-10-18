int __cdecl FUN_006d92b7(int param_1)
{
    HANDLE eventHandle = CreateEventA(nullptr, 0, 0, nullptr);
    if (eventHandle != nullptr) {
        // The member at offset 0x4 has no evidence-backed name.
        *reinterpret_cast<HANDLE*>(param_1 + 4) = eventHandle;
        return 1;
    }

    return 0;
}
