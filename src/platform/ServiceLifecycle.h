#pragma once

#include <cstdint>

namespace hp6::platform {

// 006b492b. Call slot +4 of the allocator object's virtual table.
void release_block(void* block, std::uint32_t size);

// 006b7d0f / 006b7bcb. Four component virtual calls at observed offsets.
void destroy_components(std::uint8_t* record);
void destroy_components_variant(std::uint8_t* record);

// 006b7cca / 006b7dcb. The flag's low bit requests release of the record.
std::uint8_t* destroy_record_0x84(std::uint8_t* record, std::uint8_t flag);
std::uint8_t* destroy_record_0x1c(std::uint8_t* record, std::uint8_t flag);

// 006dcbc7 / 006dcf14 / 006dcc40 / 006dce80 / 006dd111.
// A second allocator and a layered record with observed pointer offsets.
void release_secondary_block(void* block, std::uint32_t size);
void destroy_layer_core(std::uint8_t* record);
void destroy_layer_with_children(std::uint8_t* record);
std::uint8_t* destroy_layer_0xc0(std::uint8_t* record, std::uint8_t flag);
std::uint8_t* destroy_layer_0xb4(std::uint8_t* record, std::uint8_t flag);

// 006de65f / 006de70d / 006de7a1 / 006de7d0 / 006de7e5.
std::uint8_t* destroy_record_0x24(std::uint8_t* record, std::uint8_t flag);
std::uint8_t* destroy_record_0x78(std::uint8_t* record, std::uint8_t flag);
std::uint8_t* destroy_record_0x464(std::uint8_t* record, std::uint8_t flag);
void reset_record_vtable_0071b670(std::uint8_t* record);
std::uint8_t* destroy_record_0x04(std::uint8_t* record, std::uint8_t flag);

// 006de735 / 006de449 / 006de5b0 / 006de5e1 / 006de615.
void set_vtable_0071b650(std::uint8_t* record);
std::uint8_t* destroy_record_0x464_two_stage(std::uint8_t* record,
                                               std::uint8_t flag);
std::uint32_t release_member_0x468(std::uint8_t* record);
void destroy_record_with_member(std::uint8_t* record);
std::uint8_t* destroy_record_0x46c(std::uint8_t* record, std::uint8_t flag);

// 006dd00c / 006dd188 / 006dd354 / 006ddb15 (service_cleanup_038).
// The entry table sits at owner + 0x20 with its count at + 0xa0; the
// auxiliary words at + 0xa4 / + 0xa8 / + 0xac are cleared when they hold
// the removed value. Return is a bool carried in AL (confirmed against the
// exported x86; the Ghidra CONCAT31 forms are untracked-EAX artifacts).
bool remove_entry_pointer(void* owner, std::int32_t value);
void finalize_record_0071b560(std::uint8_t* record);
void finalize_record_0071b580(std::uint8_t* record);
void finalize_record_0071b5a0(std::uint8_t* record);

// 006dd2c9 / 006dd894 / 006dddf6. Two-stage destroy variants that call the
// finalizer above and, on flag bit 0, release the record of the observed
// size. Each address keeps a distinct entry.
std::uint8_t* destroy_entry_record_0071b560(std::uint8_t* record,
                                             std::uint8_t flag);
std::uint8_t* destroy_entry_record_0071b580(std::uint8_t* record,
                                             std::uint8_t flag);
std::uint8_t* destroy_entry_record_0071b5a0(std::uint8_t* record,
                                             std::uint8_t flag);

} // namespace hp6::platform
