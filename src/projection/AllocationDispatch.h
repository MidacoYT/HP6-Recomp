#pragma once
#include <cstdint>

namespace hp6::projection {

// The x86 caller supplies a six-node linked argument list to the first
// virtual function of the object stored at DAT_00895c84.
struct AllocationArgument {
    std::uint32_t index;
    std::uintptr_t value;
    AllocationArgument* next;
};

struct AllocationDispatch {
    virtual void* call(std::uint32_t size, AllocationArgument* arguments) = 0;
    virtual void release(void* address, std::uint32_t option) = 0;
};

extern AllocationDispatch* DAT_00895c84;

void release_projection_memory(void* address); // 0x006d6b3f

void* dispatch_allocation(const char* tag, std::uint32_t size,
                          std::uint32_t parameter_3, std::uint32_t parameter_4,
                          std::uint32_t parameter_5, const char* source_file,
                          std::uint32_t source_line); // 0x006d6ab6
void* create_projection_collection(const void* source,
                                   std::uint32_t context); // 0x006d7f49
void* create_clut_element(const void* source,
                          std::uint32_t context); // 0x006d7980
void* create_shp_record_variant_20(const void* source,
                                   std::uint32_t context); // 0x006d79fa
void* create_shp_record_variant_02(const void* source,
                                   std::uint32_t context); // 0x006d7d9c
void* create_clut_variant_04(const void* source,
                             std::uint32_t context); // 0x006d77dc
void* create_clip_variant_100(const void* source,
                              std::uint32_t context); // 0x006d7865
void* create_hots_variant_10(const void* source,
                             std::uint32_t context); // 0x006d78f2
void* clone_variant_02_allocated(const void* source,
                                 std::uint32_t context); // 0x006d829c
void* clone_variant_04_allocated(const void* source,
                                 std::uint32_t context); // 0x006d835e
void* clone_variant_10_allocated(const void* source,
                                 std::uint32_t context); // 0x006d84de
void* clone_variant_100_allocated(const void* source,
                                  std::uint32_t context); // 0x006d8417
void* clone_variant_80_allocated(const void* source,
                                 std::uint32_t context); // 0x006d85c9
void* clone_variant_20_allocated(const void* source,
                                 std::uint32_t context); // 0x006d8685

}
