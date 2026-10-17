#include "math/FloatAccess.h"

namespace hp6::math {

// ReAgent candidates: reports/pipeline/candidates/{004301c0,004301e0,
// 00430200,0050cc10}.cpp. Float values are exact in MSVC double precision;
// the original x87 return ABI is not reproduced by this modern interface.
long double component0(float* const* holder) {
    return static_cast<long double>((*holder)[0]);
}

long double component1(float* const* holder) {
    return static_cast<long double>((*holder)[1]);
}

long double component2(float* const* holder) {
    return static_cast<long double>((*holder)[2]);
}

long double first_float(const float* value) {
    return static_cast<long double>(*value);
}

}
