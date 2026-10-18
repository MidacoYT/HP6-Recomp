#include "render/ShaderManager.h"
#include "platform/Diagnostics.h"

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint16_t DAT_009978e8[]; // shader id free-list stack
extern std::int32_t DAT_00bb0994;    // free-list stack pointer (index)
extern std::int32_t DAT_00c754cc;    // high-water mark (ids in use)
extern std::int32_t DAT_00cf1a34;    // live shader count

namespace hp6::render {

// ReAgent candidate: reports/pipeline/candidates/01003ea0.cpp. The
// exported x86's fatal path leaves EAX as whatever
// format_diagnostic_message/vsprintf happened to return; this is
// unreachable in ordinary play (the 0x600 cap), so 0 is returned instead
// of that otherwise-unconstrained value.
std::uint32_t allocate_shader_id() {
    if (DAT_00cf1a34 >= 0x600) {
        hp6::platform::format_diagnostic_message(
            "ShaderManager::AllocateShaderID - MAXIMUM number of shaders "
            "exceeded!\n");
        return 0;
    }
    const std::uint16_t id = DAT_009978e8[DAT_00bb0994];
    ++DAT_00bb0994;
    const std::uint32_t next = static_cast<std::uint32_t>(id) + 1u;
    if (DAT_00c754cc < static_cast<std::int32_t>(next)) {
        DAT_00c754cc = static_cast<std::int32_t>(next);
    }
    ++DAT_00cf1a34;
    return (next & 0xffff0000u) | id;
}

// ReAgent's own candidate (reports/pipeline/rejected/01003f20.cpp) let
// the fatal branch fall through into the free-list bookkeeping below,
// which the exported x86 does not do: the fatal call and the normal path
// both land on the same single epilogue, so taking the fatal branch
// returns immediately. The objective verifier caught this independently
// ("P-code has 2 returns, candidate has no explicit return").
void free_shader_id(std::uint16_t id) {
    if (static_cast<std::uint32_t>(id) > static_cast<std::uint32_t>(DAT_00c754cc)) {
        hp6::platform::format_diagnostic_message(
            "ShaderManager::FreeShaderID - attempt to free a shader above the "
            "high water mark!\n");
        return;
    }
    if (static_cast<std::uint32_t>(id) + 1u == static_cast<std::uint32_t>(DAT_00c754cc)) {
        --DAT_00c754cc;
    }
    --DAT_00bb0994;
    DAT_009978e8[DAT_00bb0994] = id;
    --DAT_00cf1a34;
}

} // namespace hp6::render
