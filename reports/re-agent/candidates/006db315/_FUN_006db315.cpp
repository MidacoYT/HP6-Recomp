#include <cstdint>

struct Node_006db315 {
    Node_006db315* field_0x0;
};

struct Context_006db315 {
    Node_006db315* field_0x0;
    Node_006db315* field_0x4;
    std::uint32_t field_0x8;
};

void __thiscall FUN_006db315(void* self, Node_006db315* param_1)
{
    auto* const context = static_cast<Context_006db315*>(self);
    Node_006db315* const previous = context->field_0x4;

    param_1->field_0x0 = nullptr;
    ++context->field_0x8;
    context->field_0x4 = param_1;

    if (previous == nullptr) {
        context->field_0x0 = param_1;
    } else {
        previous->field_0x0 = param_1;
    }
}
