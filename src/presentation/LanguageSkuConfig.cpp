#include "presentation/LanguageSkuConfig.h"

namespace hp6::presentation {

LanguageSkuConfig parse_language_sku_config(std::string_view records) {
    LanguageSkuConfig result;
    std::size_t offset = 0;
    while (offset < records.size()) {
        const std::size_t end = records.find('\0', offset);
        if (end == std::string_view::npos) break;
        const std::string_view record = records.substr(offset, end - offset);
        offset = end + 1;
        if (record.starts_with("EOF")) break;
        if (record.starts_with("SKUKEY")) {
            if (record.size() >= 7) {
                result.sku_key.assign(record.substr(7, 32));
            }
        }
        if (record.starts_with("SKUCODE")) {
            if (record.size() >= 8) {
                result.sku_code.assign(record.substr(8, 32));
            }
        }
        if (record.starts_with("LANGUAGE:")) {
            result.languages.emplace_back(record.substr(9));
        }
    }
    return result;
}

} // namespace hp6::presentation
