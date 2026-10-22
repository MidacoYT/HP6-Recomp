#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"
#include "platform/HandleRegistry.h"

#include <cstdint>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint32_t DAT_00fd1b14;  // get_screen_manager_singleton's lazy-init flag
extern void* DAT_00fcd530;          // the screen manager singleton itself
extern std::uint8_t* DAT_00fcd534;  // adjacent global, read as get_screen_manager_singleton()+4

extern std::int32_t DAT_00d472dc;   // resolved chapter index; see header doc comment
extern std::uint8_t DAT_0087e014;   // "ScreenMainOSD already activated once" guard

extern std::uint32_t DAT_00fd178c;  // get_bootup_flow_singleton's lazy-init flag
extern void* DAT_00fd1718;          // the bootup-flow singleton itself

// {chapter_index, name, next_hint}[10], stride 0xc, read in live memory
// at 0x00892300 (see architecture/startup_path.md for the full dump).
struct ChapterTableEntry {
    std::int32_t index;
    const char* name;
    std::int32_t next_hint;
};
extern ChapterTableEntry DAT_00892300[10];

// 11-entry fixed chapter-name table at 0x0080a91c: NONE, BURROW_INTRO,
// HOGWARTS_1, BURROW_XMAS, HOGWARTS_2, THE_CAVE, HOGWARTS_3,
// ENDLESS_DAY, FRONT_END, LANGUAGE_SELECT, POTIONS_GAME.
extern const char* PTR_DAT_0080a91c[11];

extern void* DAT_00fcadec; // the chapter registry "this"

namespace hp6::presentation {

void* get_screen_manager_singleton() {
    if ((DAT_00fd1b14 & 1u) == 0u) {
        DAT_00fd1b14 |= 1u;
        // NOT YET INTEGRATED: thunk_FUN_00fe7f30, the real constructor.
        hp6::platform::atexit_register([] {});
    }
    return &DAT_00fcd530;
}

void emit_chapter_entry_screen_command(std::uint8_t* chapter_manager) {
    dispatch_presentation_command("Presentation.Screens.DeactivateAllScreensWithFocus");

    if (DAT_00d472dc == 8) {
        dispatch_presentation_command("OnActivateFrontEndChapter");
        get_screen_manager_singleton();
        std::memset(DAT_00fcd534 + 0xb4, 0, 0x200);
    } else if (DAT_00d472dc == 9) {
        dispatch_presentation_command(
            "Presentation.Screens.ActivateScreenWithFocus ScreenLanguageSelect");
    } else if (DAT_0087e014 == 0) {
        dispatch_presentation_command("Presentation.Screens.ActivateScreenWithFocus ScreenMainOSD");
    }

    *reinterpret_cast<std::uint32_t*>(chapter_manager + 4) &= 0xfffffffeu;
    chapter_manager[0x8c] = 0;
}

void* get_bootup_flow_singleton() {
    if ((DAT_00fd178c & 1u) == 0u) {
        DAT_00fd178c |= 1u;
        // NOT YET INTEGRATED: thunk_FUN_00fdf6b0, the real constructor.
        hp6::platform::atexit_register([] {});
    }
    return &DAT_00fd1718;
}

void resolve_and_load_chapter(void* /*chapter_registry*/, const char* chapter_name) {
    if (chapter_name == nullptr) {
        return;
    }

    ChapterTableEntry* resolved = nullptr;
    bool matched_fixed_table = false;
    for (std::int32_t fixed_index = 0; fixed_index < 11; ++fixed_index) {
        if (std::strcmp(PTR_DAT_0080a91c[fixed_index], chapter_name) != 0) {
            continue;
        }
        matched_fixed_table = true;
        if (fixed_index != 0) {
            for (auto& entry : DAT_00892300) {
                if (entry.index == fixed_index) {
                    resolved = &entry;
                    break;
                }
            }
        }
        break;
    }
    if (!matched_fixed_table || resolved == nullptr) {
        for (auto& entry : DAT_00892300) {
            if (std::strcmp(entry.name, chapter_name) == 0) {
                resolved = &entry;
                break;
            }
        }
    }

    if (resolved == nullptr) {
        // NOT YET INTEGRATED: FUN_00576f30 first calls the +0x2c slot
        // of the EventSlotGroupManager at get_bootup_flow_singleton()+4.
        // Vtable 008283d8 routes this through 0048f0f0 and the shadow
        // manager at +0x14 (vtable 00843cf8). This clears event slots;
        // it does not resolve a chapter. The native function also emits
        // a "ChapterLoading <name>" notification after this call.
        return;
    }

    DAT_00d472dc = resolved->index;
    if (DAT_00d472dc != 8) {
        // NOT YET INTEGRATED: FUN_0056d910(0xd465a0), a feature-flag clear.
    }
    if (resolved->next_hint != 0) {
        // NOT YET INTEGRATED: thunk_FUN_00fe3520.
    }
    // NOT YET INTEGRATED: the "ChapterLoading <name>" notification
    // command this function builds and dispatches afterward — it only
    // notifies, it does not change DAT_00d472dc or any state this
    // project's current tests depend on.
}

void advance_to_next_chapter() {
    // Only indices 0..8 have a following table entry (index 9 is the
    // table's last slot, POTIONS_GAME, with nothing meaningful after it
    // in memory — matching the exported x86, which never advances past
    // it either).
    for (std::int32_t i = 0; i < 9; ++i) {
        if (DAT_00892300[i].index != DAT_00d472dc) {
            continue;
        }
        const std::int32_t next_index = DAT_00892300[i + 1].index;
        DAT_00d472dc = next_index;
        if (next_index >= 0 && next_index < 11 && PTR_DAT_0080a91c[next_index] != nullptr) {
            resolve_and_load_chapter(DAT_00fcadec, PTR_DAT_0080a91c[next_index]);
        }
        return;
    }
}

} // namespace hp6::presentation
