#include "presentation/LanguageSkuConfig.h"

#include <cassert>
#include <string_view>

int main() {
    using hp6::presentation::parse_language_sku_config;
    constexpr char records[] =
        "SKUKEY:ABC\0SKUCODE:EUR\0LANGUAGE:EN-GB\0LANGUAGE:FR-FR\0"
        "EOF\0LANGUAGE:DE-DE\0";
    // Use the complete literal span; the record after EOF must be ignored.
    const auto parsed = parse_language_sku_config(
        std::string_view(records, sizeof(records) - 1));
    assert(parsed.sku_key == "ABC");
    assert(parsed.sku_code == "EUR");
    assert(parsed.languages.size() == 2);
    assert(parsed.languages[0] == "EN-GB");
    assert(parsed.languages[1] == "FR-FR");
    assert(parse_language_sku_config("LANGUAGE:EN-GB").languages.empty());
}
