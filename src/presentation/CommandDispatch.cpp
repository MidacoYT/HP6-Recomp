#include "presentation/CommandDispatch.h"

#include <cstring>

// The 93-entry command table at 0x00876ab0 (an array of pointers to
// {name, handler} pairs; see architecture/presentation_commands.csv).
// Owned by whichever translation unit links this file, since none of
// the 93 real handlers are reconstructed here.
extern const hp6::presentation::CommandTableEntry* const PTR_PTR_00876ab0[93];

// The 26-entry screen-task table at 0x00870948 (an array of pointers
// to pre-built per-screen task objects). Owned by whichever
// translation unit links this file.
extern const hp6::presentation::ScreenTaskEntry* const PTR_PTR_00870948[26];

namespace hp6::presentation {

namespace {
#ifdef _MSC_VER
using ActivateCall = std::int32_t (__thiscall*)(void*);
#else
using ActivateCall = std::int32_t (*)(void*);
#endif
} // namespace

std::int32_t find_and_invoke_command(const char* verb, const char* args) {
    for (std::size_t i = 0; i < 93; ++i) {
        const CommandTableEntry* entry = PTR_PTR_00876ab0[i];
        if (std::strcmp(verb, entry->name) == 0) {
            return entry->handler(args);
        }
    }
    return 0;
}

std::int32_t dispatch_presentation_command(const char* command) {
    char buffer[256];
    std::strncpy(buffer, command, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    char* space = std::strchr(buffer, ' ');
    const char* args = nullptr;
    if (space != nullptr) {
        *space = '\0';
        args = space + 1;
    }
    return find_and_invoke_command(buffer, args);
}

std::int32_t activate_mapped_screen_task(const char* name) {
    for (std::size_t i = 0; i < 26; ++i) {
        const ScreenTaskEntry* entry = PTR_PTR_00870948[i];
        if (std::strcmp(name, entry->name) == 0) {
            auto activate = *reinterpret_cast<ActivateCall const*>(entry->vtable);
            activate(const_cast<ScreenTaskEntry*>(entry));
            break;
        }
    }
    return 1;
}

} // namespace hp6::presentation
