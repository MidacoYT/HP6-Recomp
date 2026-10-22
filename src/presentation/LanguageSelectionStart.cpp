// 0x00505670 in the unprotected HP6 x86 PE.
// SHA-256: 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10
#include "presentation/ScreenTasks.h"
#include "presentation/CommandDispatch.h"
#include "presentation/GuiTextTable.h"

#include <cstdint>

extern void* DAT_00fcae04;      // language service, vector at +0xb8/+0xbc
extern std::uint8_t DAT_00cef53d;
extern std::uint8_t DAT_00cef53e;

namespace hp6::presentation {

std::int32_t begin_language_selection() {
    update_localized_text(0x00cf8168u, "DS_HIGHLIGHTTEXT", L"");
    const auto* service = static_cast<const std::uint8_t*>(DAT_00fcae04);
    const auto begin = *reinterpret_cast<const std::uint32_t* const*>(service + 0xb8);
    const auto end = *reinterpret_cast<const std::uint32_t* const*>(service + 0xbc);
    const auto byte_count = reinterpret_cast<std::uintptr_t>(end) -
                            reinterpret_cast<std::uintptr_t>(begin);
    DAT_00cef53e = 1;
    DAT_00cef53d = 0;
    if ((byte_count & ~std::uintptr_t{3}) == 0) return 0;
    dispatch_presentation_command("LanguageSelection");
    return 1;
}

} // namespace hp6::presentation
