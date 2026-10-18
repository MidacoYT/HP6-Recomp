// The available evidence identifies these fields by offset, but does not
// establish their original member names or enclosing types.
void __fastcall FUN_006b2199(int param_1)
{
    const int container = FUN_006b18df(
        *reinterpret_cast<const unsigned int*>(param_1 + 0x0c), 1);

    FUN_006d92a1(container + 0x28);

    const unsigned int key =
        (static_cast<unsigned int>(
             *reinterpret_cast<const unsigned char*>(param_1 + 0x12)) << 24) |
        (*reinterpret_cast<const unsigned int*>(param_1 + 0x0c) & 0x00ffff00u);

    int previous = 0;
    int current = *reinterpret_cast<const int*>(container + 0x10);

    while (current != 0 &&
           ((static_cast<unsigned int>(
                 *reinterpret_cast<const unsigned char*>(current + 0x12)) << 24) |
            (*reinterpret_cast<const unsigned int*>(current + 0x0c) & 0x00ffff00u)) <= key) {
        previous = current;
        const int next_link = *reinterpret_cast<const int*>(current + 0x08);
        current = next_link == 0 ? 0 : next_link - 8;
    }

    FUN_006b2ce9(reinterpret_cast<void*>(container + 0x10), previous, param_1);
    FUN_006d92ac(container + 0x28);
    FUN_006d92da(container + 0x44);
}
