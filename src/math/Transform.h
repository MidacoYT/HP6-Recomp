#pragma once

namespace hp6::math {

// matrix_points addresses 0..14 as a row-major float[16] block.
void transform_translated_xyz(float* output, const float* point,
                              const float* matrix_points); // 0x00404be0

// 0x00401520 (thiscall; `this` is the 16-float output). Standard
// row-major 4x4 matrix multiply: output = lhs * rhs (output[row][col] =
// sum_k lhs[row][k] * rhs[k][col]). `output` may not alias `lhs`/`rhs`.
void multiply_matrix4x4(float* output, const float* lhs, const float* rhs); // 0x00401520

// 0x005a2310 (fastcall; the destination is the implicit register
// `in_EAX` in the exported x86 -- made an explicit parameter here, the
// same way this project's other register-sourced "this"/destination
// arguments are). Transposes a 16-float row-major 4x4 matrix.
// `output` may not alias `input`.
void transpose_matrix4x4(float* output, const float* input); // 0x005a2310

}
