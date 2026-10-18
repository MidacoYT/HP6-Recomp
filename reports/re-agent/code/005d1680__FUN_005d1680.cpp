#include <cmath>

void __stdcall FUN_005d1680(float* param_1, float* param_2)
{
    const float x = param_1[0];
    const float scale = 1.0f / std::sqrt(
        x * x + param_1[1] * param_1[1]
        + param_1[2] * param_1[2]
        + param_1[3] * param_1[3]);

    param_2[0] = x * scale;
    param_2[1] = param_1[1] * scale;
    param_2[2] = scale * param_1[2];
    param_2[3] = scale * param_1[3];
}