// Generated from the original PE by tools/audit_native_globals.py.
// SHA-256: 4653e4301ed6dfc271409da2443fd817a65b6514704a3487e86895337e487954
#include <cstdint>

struct ChapterTableEntry {
    std::int32_t index;
    const char* name;
    std::int32_t next_hint;
};

const char* PTR_DAT_0080a91c[11] = {
    "NONE",
    "BURROW_INTRO",
    "HOGWARTS_1",
    "BURROW_XMAS",
    "HOGWARTS_2",
    "THE_CAVE",
    "HOGWARTS_3",
    "ENDLESS_DAY",
    "FRONT_END",
    "LANGUAGE_SELECT",
    "POTIONS_GAME",
};
ChapterTableEntry DAT_00892300[10] = {
    {9, "LanguageSelect", 0},
    {8, "FrontEnd", 0},
    {1, "BurrowIntro", 2},
    {2, "Hogwarts", 3},
    {3, "BurrowXmas", 4},
    {4, "Hogwarts", 5},
    {5, "TheCave", 6},
    {6, "Hogwarts", 7},
    {7, "Hogwarts", 8},
    {10, "Hogwarts", 0},
};
