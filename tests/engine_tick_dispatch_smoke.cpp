#include <cstdint>

std::int32_t DAT_00d63e10 = 0;
std::int32_t DAT_00d63e00 = 0;
std::int32_t DAT_00d63e08 = 0;
std::int32_t DAT_00d63df8 = 0;
std::int32_t DAT_00d63e80 = -1;
std::uint32_t DAT_00cef714 = 7;
std::uint32_t DAT_00d63e04 = 0;

namespace {
std::int32_t* handles[3] = {};
int message_count = 0;
std::uint32_t delete_record[2] = {};
int delete_count = 0;
}

std::int32_t deliver_tick_message_005db240(std::int32_t* handle) {
    if (message_count < 3) handles[message_count] = handle;
    ++message_count;
    return 1;
}
void deliver_delete_entity_005db160(const std::uint32_t* record) {
    delete_record[0] = record[0];
    delete_record[1] = record[1];
    ++delete_count;
}
void dispatch_engine_tick_005dcb30();

int main() {
    DAT_00d63e10 = 10;
    DAT_00d63e00 = DAT_00d63e80;
    DAT_00d63e08 = 30;
    DAT_00d63df8 = 40;
    dispatch_engine_tick_005dcb30();
    if (message_count != 2 || handles[0] != &DAT_00d63e10 ||
        handles[1] != &DAT_00d63e08 || delete_count != 1 ||
        delete_record[0] != 40 || delete_record[1] != 0 ||
        DAT_00d63e04 != 7 || DAT_00cef714 != 8) return 1;

    DAT_00d63e10 = DAT_00d63e80;
    DAT_00d63e08 = DAT_00d63e80;
    DAT_00d63df8 = DAT_00d63e80;
    dispatch_engine_tick_005dcb30();
    return message_count == 2 && delete_count == 1 &&
           DAT_00d63e04 == 8 && DAT_00cef714 == 9 ? 0 : 2;
}
