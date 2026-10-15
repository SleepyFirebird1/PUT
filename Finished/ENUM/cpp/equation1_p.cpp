#define MPFR_USE_NO_MACRO
#define MPFR_USE_INTMAX_T

#include "interval.h"
#include "mpreal.h"
#include <cmath>

using namespace interval_arithmetic;

// Równanie sin(x)^2 + sin(x)/2 - 0.5 //

// Eksportowanie funkcji równania i pochodnych
extern "C" __attribute__((visibility("default"))) Interval<mpreal> equation(const Interval<mpreal>& x) {
    return ISin(x) * ISin(x) + ISin(x) / Interval<mpreal>(mpreal(2), mpreal(2)) - Interval<mpreal>(mpreal(0.5), mpreal(0.5)); // pow(sin(x), 2) + sin(x)/2 - 0.5
}

extern "C" __attribute__((visibility("default"))) Interval<mpreal> first_derivative(const Interval<mpreal>& x) {
    return Interval<mpreal>(mpreal(2), mpreal(2)) * ISin(x) * ICos(x) + ICos(x) / Interval<mpreal>(mpreal(2), mpreal(2)); // 2*sin(x)*cos(x) + cos(x)/2
}

extern "C" __attribute__((visibility("default"))) Interval<mpreal> second_derivative(const Interval<mpreal>& x) {
    return Interval<mpreal>(mpreal(2), mpreal(2)) * ICos(Interval<mpreal>(mpreal(2), mpreal(2)) * x) - ISin(x) / Interval<mpreal>(mpreal(2), mpreal(2)); // 2*cos(2*x) - sin(x)/2
}