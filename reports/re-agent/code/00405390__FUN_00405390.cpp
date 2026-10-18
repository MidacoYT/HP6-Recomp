void __cdecl FUN_00405390(float* param_1, float* param_2, float* param_3)
{
    const float a0 = param_2[0];
    const float a1 = param_2[1];
    const float a2 = param_2[2];
    const float a3 = param_2[3];

    const float b0 = param_3[0];
    const float b1 = param_3[1];
    const float b2 = param_3[2];
    const float b3 = param_3[3];

    const float r0 = a0 - b0;
    const float r1 = a1 - b1;
    const float r2 = a2 - b2;
    const float r3 = a3 - b3;

    param_1[0] = r0;
    param_1[1] = r1;
    param_1[2] = r2;
    param_1[3] = r3;
}