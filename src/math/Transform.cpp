#include "math/Transform.h"

namespace hp6::math {

// ReAgent candidate: reports/pipeline/candidates/00404be0.cpp
void transform_translated_xyz(float* output, const float* point,
                              const float* matrix_points) {
    const float x = point[0] - matrix_points[12];
    const float y = point[1] - matrix_points[13];
    const float z = point[2] - matrix_points[14];
    output[0] = matrix_points[1] * y + matrix_points[2] * z
              + matrix_points[3] * 0.0f + matrix_points[0] * x;
    output[1] = matrix_points[5] * y + matrix_points[6] * z
              + matrix_points[7] * 0.0f + matrix_points[4] * x;
    output[2] = matrix_points[9] * y + matrix_points[10] * z
              + matrix_points[11] * 0.0f + matrix_points[8] * x;
}

void multiply_matrix4x4(float* output, const float* lhs, const float* rhs) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            output[row * 4 + col] = lhs[row * 4 + 0] * rhs[0 * 4 + col] +
                                    lhs[row * 4 + 1] * rhs[1 * 4 + col] +
                                    lhs[row * 4 + 2] * rhs[2 * 4 + col] +
                                    lhs[row * 4 + 3] * rhs[3 * 4 + col];
        }
    }
}

void transpose_matrix4x4(float* output, const float* input) {
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            output[col * 4 + row] = input[row * 4 + col];
        }
    }
}

}
