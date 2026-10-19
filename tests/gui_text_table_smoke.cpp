// Local behavior checks for the shared localized-text map
// (update_localized_text, 0x00429430, backed by the fixed table at
// 0x00cf8168; see GuiTextTable.h and
// architecture/startup_path.md). The reconstruction keeps the
// observable insert/replace/null-guard contract, so the checks observe
// through the documented test accessor instead of claiming the
// binary's internal node layout.
#include "presentation/GuiTextTable.h"

#include <cstdint>
#include <cstdio>
#include <cwchar>

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);
    clear_localized_text_tables();

    // 0x00429430's own null guard: null key or null text silently
    // does nothing.
    update_localized_text(0x00cf8168u, nullptr, L"anything");
    update_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0", nullptr);
    expect("null_key_is_noop",
           read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0") == nullptr);

    // Insert.
    update_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0", L"English");
    const wchar_t* stored = read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0");
    expect("inserted_value_readable",
           stored != nullptr && std::wcscmp(stored, L"English") == 0);

    // Replace under the same key (the exported x86's insert-or-update
    // behavior).
    update_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0", L"German");
    stored = read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0");
    expect("replaced_value_readable",
           stored != nullptr && std::wcscmp(stored, L"German") == 0);

    // Tables are independent (the binary roots one structure per table
    // address).
    update_localized_text(0x00cf816cu, "GUI_LANGUAGEMENU0", L"Francais");
    expect("second_table_independent",
           std::wcscmp(read_localized_text(0x00cf816cu, "GUI_LANGUAGEMENU0"),
                       L"Francais") == 0 &&
               std::wcscmp(read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0"),
                           L"German") == 0);

    // Untouched keys read back null.
    expect("unknown_key_reads_null",
           read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU7") == nullptr);

    // Clear returns to the game's freshly-booted empty state.
    clear_localized_text_tables();
    expect("clear_empties_first_table",
           read_localized_text(0x00cf8168u, "GUI_LANGUAGEMENU0") == nullptr);
    expect("clear_empties_second_table",
           read_localized_text(0x00cf816cu, "GUI_LANGUAGEMENU0") == nullptr);

    if (failures != 0) {
        std::printf("gui_text_table_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("gui_text_table_smoke: all checks passed\n");
    return 0;
}
