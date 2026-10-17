#pragma once

namespace hp6::math {

// matrix_points addresses 0..14 as a row-major float[16] block.
void transform_translated_xyz(float* output, const float* point,
                              const float* matrix_points); // 0x00404be0

}
