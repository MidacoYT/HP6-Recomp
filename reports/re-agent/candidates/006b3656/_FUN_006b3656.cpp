int __stdcall FUN_006b3656(void)
{
    const DWORD driveMask = GetLogicalDrives();
    FUN_006b3b0e(&DAT_00892b80, 0x68);

    auto* output = reinterpret_cast<std::uint32_t*>(&DAT_00892b80);
    const auto* entries = reinterpret_cast<const std::uint32_t*>(&PTR_DAT_008513e0);

    int count = 0;
    for (int index = 0; index < 0x1a; ++index) {
        if ((driveMask & (1u << index)) != 0) {
            output[count] = entries[index];
            ++count;
        }
    }

    return count;
}
