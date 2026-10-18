#pragma once
#include <cstdint>

namespace hp6::projection {

void set_relative_payload(void* record, const void* payload,
                          std::uint32_t size); // 0x00688550
void link_next_record(void* record, const void* next); // 0x006d6bf1
void decode_variant_02(const void* source, void* destination); // 0x006d7cd2
void decode_variant_100(const void* source, void* destination); // 0x006d7827
void decode_variant_20(const void* source, void* destination); // 0x006d79ce
void decode_variant_04(const void* source, void* destination); // 0x006d778a
void decode_variant_10(const void* source, void* destination); // 0x006d78a8
void clone_variant_02(const void* source, void* destination); // 0x006d822a
void decode_variant_80(const void* source, void* destination); // 0x006d793d
void assemble_projection_records(void* storage, const void* source); // 0x006d7dea

// --- Constructeurs d'enregistrements (appuient sur le sous-systeme decode) ---

// Invoque un rappel via un pointeur de fonction stocke dans le global DAT_00895c84.
// Les valeurs 1..6 sont preparees sur la pile mais seul le compteur (6) est transmis.
// Adresse globale externe: 0x00895c84.
void invoke_global_callback(void* param1, void* param2, void* param3, void* param4,
                            void* param5, void* param6, void* param7); // 0x006d6ab6

// Construit un enregistrement de type 0x69 (variante 80) : en-tête, payload relatif,
// puis ecrit "EAGL<taille>" via sprintf dans la zone payload.
void construct_variant_80_record(void* this_, int source); // 0x006d793d

// Cree un element CLUT : invoque le callback global, puis construit un enregistrement
// variant 80, et positionne le bit 10 du premier mot. Retourne le pointeur this.
std::uint32_t* create_clut_element(int param_1, std::uint32_t param_2); // 0x006d7980

// Cree un enregistrement cr_tex via decode_variant_20 : invoque le callback global,
// puis decode la variante 20, et positionne le bit 10.
std::uint32_t* create_shp_record_variant_20(int param_1, std::uint32_t param_2); // 0x006d79fa

// Cree un enregistrement cr_tex via decode_variant_02 avec estimation de taille :
// invoque le callback global, puis decode la variante 02, et positionne le bit 10.
std::uint32_t* create_shp_record_variant_02(int param_1, std::uint32_t param_2); // 0x006d7d9c

}
