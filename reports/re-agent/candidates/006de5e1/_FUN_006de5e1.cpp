#include <cstddef>
#include <cstdint>

struct FUN_006de5e1_Object {
    void* vtable;
    std::byte unresolved_004[0x464];
    std::int32_t field_468; // Member name and concrete type remain unresolved.
};

void __fastcall FUN_006de5e1(FUN_006de5e1_Object* self)
{
    self->vtable = &PTR_FUN_0071b608;
    FUN_006de5b0(reinterpret_cast<std::int32_t>(self));

    const std::int32_t value = self->field_468;
    if (value != 0) {
        FUN_005dc710(value);
        self->field_468 = 0;
    }

    self->vtable = &PTR_FUN_0071b5e8;
    FUN_006de735(self);
}
