#pragma once

#ifdef ANDROID
#include <cmath>
#endif

namespace Utility
{
    void rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& brightness);

    template <typename T>
        inline T lerp(T v0, T v1, T t) {
        return fma(t, v1, fma(-t, v0, v0));
    }
}

#ifdef ANDROID
#define round _huemath::_round
#define log2 _huemath::_log2

namespace _huemath
{
    inline int _round(double a)
    {
        return static_cast<int>(a + 0.5);
    }

    inline double _log2(double a)
    {
        return log(a) / log(2);
    }
}
#endif
