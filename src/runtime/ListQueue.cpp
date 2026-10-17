#include "runtime/ListQueue.h"

#include <cstddef>
#include <cstdint>

namespace hp6::runtime {
namespace {

std::uint32_t& word(std::uint8_t* bytes, std::size_t offset) {
    return *reinterpret_cast<std::uint32_t*>(bytes + offset);
}

std::uint8_t* pointer(std::uint32_t address) {
    return reinterpret_cast<std::uint8_t*>(address);
}

std::uint32_t address(const std::uint8_t* bytes) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(bytes));
}

} // namespace

void queue_push_006db315(std::uint8_t* header, std::uint8_t* node) {
    auto* tail = pointer(word(header, 4));
    word(node, 0) = 0;
    ++word(header, 8);
    word(header, 4) = address(node);
    if (tail == nullptr) {
        word(header, 0) = address(node);
    } else {
        word(tail, 0) = address(node);
    }
}

void queue_pop_006db335(std::uint8_t* header) {
    auto* first = pointer(word(header, 0));
    if (first == nullptr) {
        return;
    }
    if (address(first) == word(header, 4)) {
        word(header, 4) = 0;
        word(header, 0) = 0;
    } else {
        word(header, 0) = word(first, 0);
    }
    --word(header, 8);
    word(first, 0) = 0;
}

bool queue_remove_006b2dde(std::uint8_t* header, std::uint8_t* node,
                           std::uint8_t* previous_hint) {
    auto* head = pointer(word(header, 0));
    if (head == node) {
        --word(header, 8);
        if (address(node) == word(header, 4)) {
            word(header, 4) = 0;
            word(header, 0) = 0;
        } else {
            word(header, 0) = word(node, 0);
        }
        word(node, 0) = 0;
        return true;
    }
    if (head == nullptr) {
        return false;
    }
    auto* previous = previous_hint != nullptr ? previous_hint : head;
    // 006b2dde 2e15..2e2d: the search tests [previous] for zero BEFORE
    // dereferencing again, so it never walks into a null slot.
    for (;;) {
        if (word(previous, 0) == 0) {
            return false;
        }
        auto* candidate = pointer(word(previous, 0));
        if (candidate == node) {
            break;
        }
        previous = candidate;
    }
    --word(header, 8);
    word(previous, 0) = word(node, 0);
    if (address(node) == word(header, 4)) {
        word(header, 4) = address(previous);
    }
    word(node, 0) = 0;
    return true;
}

} // namespace hp6::runtime
