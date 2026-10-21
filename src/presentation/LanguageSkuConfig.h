#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace hp6::presentation {

struct LanguageSkuConfig {
    std::string sku_key;
    std::string sku_code;
    std::vector<std::string> languages;
};

// 0x005e5f90 consumes NUL-separated records and stops at an "EOF" record.
// LANGUAGE: entries are appended in source order, which determines the
// GUI_LANGUAGEMENU0..15 labels populated by 0x00429890.
LanguageSkuConfig parse_language_sku_config(std::string_view records);

} // namespace hp6::presentation
