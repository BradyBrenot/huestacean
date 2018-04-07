#pragma once

#include <cmath>

namespace Color
{
	//
	// Color space conversions
	//

	//Convert RGB to CIE XYZ, optionally performing gamma correction.
	//Using formulas from Philips Hue documentation.
	template<bool doGammaCorrection = true>
	void rgb_to_XYZ(double& r, double& g, double& b, double& X, double& Y, double& Z)
	{
		double R, G, B;

#if __cplusplus >= 201703L
		if constexpr(doGammaCorrection)
#else
		if(doGammaCorrection)
#endif
		{
			//Gamma correction, per hue docs
			R = (r > 0.04045f) ? pow((r + 0.055f) / (1.0f + 0.055f), 2.4f) : (r / 12.92f);
			G = (g > 0.04045f) ? pow((g + 0.055f) / (1.0f + 0.055f), 2.4f) : (g / 12.92f);
			B = (b > 0.04045f) ? pow((b + 0.055f) / (1.0f + 0.055f), 2.4f) : (b / 12.92f);
		}
		else
		{
			R = r; G = g; B = b;
		}

		X = R * 0.664511f + G * 0.154324f + B * 0.162028f;
		Y = R * 0.283881f + G * 0.668433f + B * 0.047685f;
		Z = R * 0.000088f + G * 0.072310f + B * 0.986039f;
	}

	//Convert CIE XYZ to to CIE LCh (expensive)
	// (cylindrical coordinate conversion of CIE L*a*b*, where L* is unchanged, C* is chromaticity, and h° is hue)
	void XYZ_to_LCh(double& X, double& Y, double& Z, double& L, double& C, double& h);

	//Convert CIE LCh to CIE XYZ (expensive)
	void LCh_to_XYZ(double& L, double& C, double& h, double& X, double& Y, double& Z);

	//Convert CIE XYZ to CIE xyY
	void XYZ_to_xy(double& X, double& Y, double& Z, double& x, double& y);

	//Convert RGB straight to CIE xyY
	void rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& Y);

	//
	// Color constants
	//

	// CIE 1931 coordinates of CIE Standard Illuminant D65
	constexpr double D65_x = 0.31271;
	constexpr double D65_y = 0.32902;

	// Normalized XYZ coordinates of D65 (normalized s.t. Y = 100)
	constexpr double D65_Xn = 95.047;
	constexpr double D65_Yn = 100.0;
	constexpr double D65_Zn = 108.883;
}

namespace Utility
{
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
