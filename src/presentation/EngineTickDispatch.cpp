#include <cstdint>

extern std::int32_t DAT_00d63e10;
extern std::int32_t DAT_00d63e00;
extern std::int32_t DAT_00d63e08;
extern std::int32_t DAT_00d63df8;
extern std::int32_t DAT_00d63e80;
extern std::uint32_t DAT_00cef714;
extern std::uint32_t DAT_00d63e04;

// The first callee is 0x005DB240. The last is 0x005DB160 (EAX carries a
// two-word stack record in the x86 code). See EngineMessageDelivery.cpp.
extern std::int32_t deliver_tick_message_005db240(std::int32_t* handle);
extern void deliver_delete_entity_005db160(const std::uint32_t* record);

void dispatch_engine_tick_005dcb30() {
    // 0x005DCB30 jumps to 0x00FF8D50 in the unprotected executable.
    if (DAT_00d63e10 != DAT_00d63e80) {
        deliver_tick_message_005db240(&DAT_00d63e10);
    }
    DAT_00d63e04 = DAT_00cef714;
    ++DAT_00cef714;
    if (DAT_00d63e00 != DAT_00d63e80) {
        deliver_tick_message_005db240(&DAT_00d63e00);
    }
    if (DAT_00d63e08 != DAT_00d63e80) {
        deliver_tick_message_005db240(&DAT_00d63e08);
    }
    if (DAT_00d63df8 != DAT_00d63e80) {
        const std::uint32_t record[2] = {
            static_cast<std::uint32_t>(DAT_00d63df8), 0
        };
        deliver_delete_entity_005db160(record);
    }
}
