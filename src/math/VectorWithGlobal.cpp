#include "math/Scalar.h"
#include "math/Vector.h"

namespace hp6::math {

// Address 0x007db234 is read by the binary. Its stored value is not present
// in the exported globals, so the final binary must supply it explicitly.
extern float DAT_007db234;

// ReAgent candidate: reports/pipeline/candidates/005d1580.cpp
void normalize_xyz_preserve_w(const float* input, float* output) {
    const float y = input[1];
    const float w = input[3];
    const float x = input[0];
    const float z = input[2];
    const long double length = sqrt_x87(y * y + x * x + z * z);
    float scale = static_cast<float>(length);
    if (length != 0.0L) {
        scale = DAT_007db234 / scale;
    }
    output[0] = x * scale;
    output[1] = y * scale;
    output[2] = scale * z;
    output[3] = w;
}

}
