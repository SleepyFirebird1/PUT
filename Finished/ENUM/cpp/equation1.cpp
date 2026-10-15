#include <quadmath.h>
#include <cmath>

// Równanie sin(x)^2 + sin(x)/2 - 0.5 //
extern "C" __attribute__((visibility("default"))) __float128 equation(__float128 x) {
    return powq(sinq(x), 2) + sinq(x) / 2 - 0.5Q;
}

extern "C" __attribute__((visibility("default"))) __float128 first_derivative(__float128 x) {
    return 2 * sinq(x) * cosq(x) + cosq(x) / 2;
}

extern "C" __attribute__((visibility("default"))) __float128 second_derivative(__float128 x) {
    return 2 * cosq(2 * x) - sinq(x) / 2;
}