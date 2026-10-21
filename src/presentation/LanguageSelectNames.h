#pragma once

#include <array>

namespace hp6::presentation {

struct LanguageSelectName {
    const char* locale;
    const wchar_t* display;
};

// 0x005e5480 fills the locale service's name-to-wide-label map from these
// UTF-16 literals in hp6.exe SHA-256
// 4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954,
// file offsets 0x43f26c..0x43f3f0. The executable's table has 21 entries;
// these sixteen correspond to the localized archives installed here. The
// slot order below follows the supplied screenshot of the original menu.
// A direct trace of the service's installed-language vector is pending.
inline constexpr std::array<LanguageSelectName, 16> kInstalledLanguageNames{{
    {"en-gb", L"English"},
    {"fr-fr", L"Fran\u00e7ais"},
    {"it-it", L"Italiano"},
    {"de-de", L"Deutsch"},
    {"es-es", L"Espa\u00f1ol"},
    {"pt-pt", L"Portugu\u00eas"},
    {"nl-nl", L"Nederlands"},
    {"pl-pl", L"Polski"},
    {"ru-ru", L"Pycc\u03ba\u0438\u0439"},
    {"da-dk", L"Dansk"},
    {"sv-se", L"Svenska"},
    {"hu-hu", L"Magyar"},
    {"cs-cz", L"\u010cesky"},
    {"pt-br", L"Portugu\u00eas (Brasil)"},
    {"fi-fi", L"Suomi"},
    {"no-no", L"Norsk"},
}};

} // namespace hp6::presentation
