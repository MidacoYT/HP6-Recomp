#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

namespace {
int cleanup_calls = 0;
int release_calls = 0;
int free_calls = 0;
void* last_pointer = nullptr;
std::uint32_t last_mode = 0;
} // namespace

void cleanup_service_holder_005db9a0(void* holder) {
    if (release_calls != 0) cleanup_calls = -100;
    last_pointer = holder;
    ++cleanup_calls;
}

void service_handle_free_005db8d0(void* block, std::uint32_t mode) {
    last_pointer = block;
    last_mode = mode;
    ++free_calls;
}

namespace hp6::platform {
void release_through_service(void* object) {
    if (cleanup_calls <= 0) release_calls = -100;
    last_pointer = object;
    ++release_calls;
}
} // namespace hp6::platform

int main() {
    hp6::platform::ServiceHolderObject holder{};
    if (holder.destroy(0) != &holder || cleanup_calls != 1 ||
        release_calls != 0 || last_pointer != &holder) return 1;
    if (holder.destroy(2) != &holder || cleanup_calls != 2 ||
        release_calls != 0) return 2;
    if (holder.destroy(1) != &holder || cleanup_calls != 3 ||
        release_calls != 1 || last_pointer != &holder) return 3;
    auto* secondary = reinterpret_cast<hp6::platform::ServiceHolderSecondary*>(
        reinterpret_cast<std::uint8_t*>(&holder) + 4);
    secondary->free_block(&holder, 7);
    if (free_calls != 1 || last_pointer != &holder || last_mode != 7) return 4;
    cleanup_calls = 0;
    release_calls = 0;
    if (secondary->destroy(0) != &holder || cleanup_calls != 1 ||
        release_calls != 0 || last_pointer != &holder) return 5;
    return 0;
}
