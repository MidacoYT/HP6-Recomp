#include "presentation/GuiTextTable.h"

#include <string>
#include <unordered_map>

namespace hp6::presentation {
namespace {

// The exported x86 (FUN_00429430) keeps one hand-rolled hash/linked-list
// structure rooted at each table address; nothing integrated in this
// project reads its internal node layout, so the observable per-table
// key/value contract is kept over plain standard maps instead (see
// GuiTextTable.h for the boundary this draws and why).
std::unordered_map<std::uint32_t,
                   std::unordered_map<std::string, std::wstring>>&
text_tables() {
    static std::unordered_map<std::uint32_t,
                              std::unordered_map<std::string, std::wstring>>
        tables;
    return tables;
}

} // namespace

void update_localized_text(std::uint32_t table, const char* key,
                           const wchar_t* text) {
    if (key == nullptr || text == nullptr) {
        // The exported x86's own `if (param_2 == 0 || param_3 == 0)
        // return;` guard: a null key or text silently does nothing.
        return;
    }
    text_tables()[table].insert_or_assign(std::string(key),
                                          std::wstring(text));
}

const wchar_t* read_localized_text(std::uint32_t table, const char* key) {
    if (key == nullptr) {
        return nullptr;
    }
    const auto& strings = text_tables()[table];
    const auto found = strings.find(key);
    return found == strings.end() ? nullptr : found->second.c_str();
}

void clear_localized_text_tables() { text_tables().clear(); }

} // namespace hp6::presentation
