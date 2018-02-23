#include "utility.h"

#include <algorithm>
#include <cmath>

/*
 * Color conversion per Hue docs, _except_ for brightness.
 * Apparent brightness (Y) seems to generally be too dim.
 */
void Utility::rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& brightness)
{
    brightness = std::max(std::max(r, g), b);

    r = (r > 0.04045f) ? pow((r + 0.055f) / (1.0f + 0.055f), 2.4f) : (r / 12.92f);
    g = (g > 0.04045f) ? pow((g + 0.055f) / (1.0f + 0.055f), 2.4f) : (g / 12.92f);
    b = (b > 0.04045f) ? pow((b + 0.055f) / (1.0f + 0.055f), 2.4f) : (b / 12.92f);

    double X = r * 0.664511f + g * 0.154324f + b * 0.162028f;
    double Y = r * 0.283881f + g * 0.668433f + b * 0.047685f;
    double Z = r * 0.000088f + g * 0.072310f + b * 0.986039f;

    x = X / (X + Y + Z);
    y = Y / (X + Y + Z);

    //brightness = Y;
}