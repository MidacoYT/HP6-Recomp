#pragma once
#include <cstdint>

namespace hp6::render {

// 0x01003ea0 (stdcall). Allocates the next free shader id from the
// DAT_009978e8 free-list stack (read at DAT_00bb0994, which is then
// incremented) while DAT_00cf1a34 (the live count) stays below 0x600;
// raises DAT_00c754cc (the high-water mark) when the allocated id reaches
// it. Returns the exported x86's own 32-bit value (CONCAT22 of the high
// word of id+1 and the id itself in the low word) exactly, even though
// the high word is 0 for every id below 0xffff. Calls the fatal
// diagnostic "ShaderManager::AllocateShaderID - MAXIMUM number of shaders
// exceeded!" and returns 0 once the 0x600 cap is reached.
std::uint32_t allocate_shader_id();

// 0x01003f20 (cdecl). Returns `id` to the DAT_009978e8 free-list stack
// (decrementing DAT_00bb0994 first) and decrements the live count
// DAT_00cf1a34; lowers the high-water mark DAT_00c754cc when `id` was the
// top of the mark. When `id` is above the current high-water mark, calls
// the fatal diagnostic "ShaderManager::FreeShaderID - attempt to free a
// shader above the high water mark!" and returns immediately, touching
// none of those globals — the exported x86 has that call and the normal
// path converge on the same single epilogue, so the two are mutually
// exclusive.
void free_shader_id(std::uint16_t id);

} // namespace hp6::render
