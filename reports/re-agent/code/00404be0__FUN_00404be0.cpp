void __fastcall FUN_00404be0(float* param_1, float* param_2, float* param_3)
{
    const float fVar1 = param_2[0] - param_3[12];
    const float fVar2 = param_2[1] - param_3[13];
    const float fVar3 = param_2[2] - param_3[14];

    param_1[0] = param_3[1] * fVar2 + param_3[2] * fVar3
               + param_3[3] * 0.0f + param_3[0] * fVar1;
    param_1[1] = param_3[5] * fVar2 + param_3[6] * fVar3
               + param_3[7] * 0.0f + param_3[4] * fVar1;
    param_1[2] = param_3[9] * fVar2 + param_3[10] * fVar3
               + param_3[11] * 0.0f + param_3[8] * fVar1;
}