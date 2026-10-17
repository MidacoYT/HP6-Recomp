#pragma once

namespace hp6::math {

// The first three functions dereference a pointer stored at *holder.
// Their caller 0x0042fa90 uses them together; no owner class is established.
long double component0(float* const* holder); // 0x004301c0
long double component1(float* const* holder); // 0x004301e0
long double component2(float* const* holder); // 0x00430200
long double first_float(const float* value);  // 0x0050cc10

}
