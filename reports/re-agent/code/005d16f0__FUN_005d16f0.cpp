void __stdcall FUN_005d16f0(float* param_1, float* param_2, float* param_3)
{
    const float fVar1 = param_1[0];
    const float fVar2 = param_1[1];
    const float fVar3 = param_1[2];
    const float fVar4 = param_2[1];
    const float fVar5 = param_2[2];
    const float fVar6 = param_2[0];

    param_3[2] = fVar4 * fVar1 - fVar6 * fVar2;
    param_3[0] = fVar5 * fVar2 - fVar4 * fVar3;
    param_3[1] = fVar6 * fVar3 - fVar5 * fVar1;

    // The binary copies an uninitialized stack float into this slot.
    float local_14;
    param_3[3] = local_14;
}