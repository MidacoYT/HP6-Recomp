#include <cstdint>
#include <cstring>

std::int32_t DAT_00d63e80 = 0;
int flush_calls = 0;
void* flushed_header = nullptr;
void flush_deferred_listeners_00ff87d0(void* header) {
    ++flush_calls;
    flushed_header = header;
}

std::int32_t deliver_tick_message_005db240(std::int32_t* handle);
void deliver_delete_entity_005db160(const std::uint32_t* record);

namespace {
struct Listener {
    void** vtable;
    void* header;
    int notify_calls = 0;
    int delete_calls = 0;

    void on_delete(std::uint32_t flag) {
        if (flag == 1) ++delete_calls;
        *static_cast<void**>(header) = header;
    }
    void on_notify(std::int32_t*) { ++notify_calls; }
};

template <typename Method>
void* method_address(Method value) {
    union { Method method; void* address; } conversion{};
    conversion.method = value;
    return conversion.address;
}

struct Node {
    void* next;
    void* previous;
    Listener* listener;
};
} // namespace

int main() {
    if (sizeof(void*) != 4) return 0;
    void* vtable[2] = {
        method_address(&Listener::on_delete),
        method_address(&Listener::on_notify)
    };
    alignas(4) std::uint8_t header[0x14] = {};
    Listener first{vtable, header};
    Listener second{vtable, header};
    Node a{nullptr, nullptr, &first};
    Node b{nullptr, nullptr, &second};
    a.next = &b;
    b.next = header;
    void* head = &a;
    std::memcpy(header, &head, 4);
    header[0x12] = 1;
    auto handle = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(header));

    if (deliver_tick_message_005db240(&handle) != 1 ||
        first.notify_calls != 1 || second.notify_calls != 1 ||
        header[0x12] != 0 || header[0x13] != 0 ||
        flush_calls != 1 || flushed_header != header) return 1;

    head = header;
    std::memcpy(header, &head, 4);
    if (deliver_tick_message_005db240(&handle) != 0) return 2;
    a.next = header;
    head = &a;
    std::memcpy(header, &head, 4);
    const std::uint32_t record[2] = {
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(header)), 0
    };
    deliver_delete_entity_005db160(record);
    return first.delete_calls == 1 && *reinterpret_cast<void**>(header) == header ? 0 : 3;
}
