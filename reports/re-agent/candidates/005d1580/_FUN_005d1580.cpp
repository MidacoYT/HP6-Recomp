extern float DAT_007db234;
long double __cdecl FUN_00401020(float param_1);

void __stdcall FUN_005d1580(float* param_1, float* param_2)
{
    const float y = param_1[1];
    const float w = param_1[3];
    const float x = param_1[0];
    const float z = param_1[2];

    const long double length = FUN_00401020(y * y + x * x + z * z);
    float scale = static_cast<float>(length);
    if (length != 0.0L) {
        scale = DAT_007db234 / scale;
    }

    param_2[0] = x * scale;
    param_2[1] = y * scale;
    param_2[2] = scale * z;
    param_2[3] = w;
}
