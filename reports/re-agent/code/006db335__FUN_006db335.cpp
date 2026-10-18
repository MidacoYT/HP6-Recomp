#include <cstdint>

struct Node_006db335 {
    std::uint32_t link;
};

struct List_006db335 {
    std::uint32_t head;
    std::uint32_t tail;
    std::uint32_t count;
};

// The observed layout uses 32-bit fields and a 32-bit target ABI.
void __fastcall FUN_006db335(List_006db335* param_1)
{
    auto* node = reinterpret_cast<Node_006db335*>(param_1->head);

    if (node != nullptr) {
        auto* nodeToClear = node;

        if (node == reinterpret_cast<Node_006db335*>(param_1->tail)) {
            param_1->tail = 0;
            param_1->head = 0;
        } else {
            const std::uint32_t nextLink = node->link;
            param_1->head = nextLink;
            nodeToClear = reinterpret_cast<Node_006db335*>(nextLink);
        }

        param_1->count = param_1->count - 1;
        nodeToClear->link = 0;
    }
}