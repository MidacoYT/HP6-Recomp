#include <cstring>

// The writable prefix starts at 0x00CEF6C0 and the next service global is
// at 0x00CEF700, leaving 0x40 bytes. Both verified PEs initialize it to zero.
char DAT_00cef6c0[0x40] = {};

// 0x005DB680 jumps to 0x00FF88F0 in the unprotected x86 executable.
// When the prefix is empty the original returns the name directly; otherwise
// it writes prefix + "::" + name into the caller's 0x100-byte stack buffer.
const char* service_context_name_005db680(char* destination,
                                         const char* name) {
    if (DAT_00cef6c0[0] == '\0') return name;
    std::strcpy(destination, DAT_00cef6c0);
    std::strcat(destination, "::");
    std::strcat(destination, name);
    return destination;
}
