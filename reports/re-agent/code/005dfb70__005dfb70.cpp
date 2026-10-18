// The allocator's signature and the three globals' owning type are unresolved.
std::uint32_t* __stdcall FUN_005dfb70()
{
    DAT_00d63ee0 = static_cast<std::uint32_t*>(
        FUN_005dc750(0x400, "RenderWare::EventHandler::LinkedMsg"));

    std::uint32_t* const end = DAT_00d63ee0 + 0x100;
    _DAT_00d63ee4 = end;
    DAT_00d63ee8 = DAT_00d63ee0;

    for (std::uint32_t* current = DAT_00d63ee0;
         current != end;
         current += 4) {
        *current = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(current + 4));
    }

    return reinterpret_cast<std::uint32_t*>(&DAT_00d63ee0);
}