#pragma once

namespace Utility
{
    void rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& brightness);

    template <typename T>
        inline T lerp(T v0, T v1, T t) {
        return fma(t, v1, fma(-t, v0, v0));
    }
}