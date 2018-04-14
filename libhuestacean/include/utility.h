#pragma once

#include <cmath>
#include <algorithm>
#include <QDebug>

namespace Color
{
	//
	// Color space conversions
	//

	//Convert CIE XYZ to to CIE LCh (expensive)
	// (cylindrical coordinate conversion of CIE L*a*b*, where L* is unchanged, C* is chromaticity, and h° is hue)
	void XYZ_to_LCh(double& X, double& Y, double& Z, double& L, double& C, double& h);

	//Convert CIE LCh to CIE XYZ (expensive)
	void LCh_to_XYZ(double& L, double& C, double& h, double& X, double& Y, double& Z);

	//Convert CIE XYZ to CIE xyY
	void XYZ_to_xy(double& X, double& Y, double& Z, double& x, double& y);

	//Convert RGB straight to CIE xyY
	void rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& Y);

	//Fit x,y into hue gamut
	void FitInGamut(double& x, double& y);

	//Convert RGB to CIE XYZ, optionally performing gamma correction.
	//Using formulas from Philips Hue documentation.
	template<bool doGammaCorrection = true>
	void rgb_to_XYZ(double& r, double& g, double& b, double& X, double& Y, double& Z)
	{
		double R, G, B;

#if __cplusplus >= 201703L
		if constexpr(doGammaCorrection)
#else
		if (doGammaCorrection)
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

		double rawX = R * 0.664511f + G * 0.154324f + B * 0.162028f;
		double rawY = R * 0.283881f + G * 0.668433f + B * 0.047685f;
		double rawZ = R * 0.000088f + G * 0.072310f + B * 0.986039f;

#if 0
		//IEC 61966-2-1:1999
		X = R * 0.4124 + G * 0.3576 + B * 0.1805;
		Y = R * 0.2126 + G * 0.7125 + B * 0.0722;
		Z = R * 0.0193 + G * 0.1192 + B * 0.9505;
#endif

		if (rawX + rawY + rawZ == 0)
		{
			X = rawX; Y = rawY; Z = rawZ;
			return;
		}

		//Renormalize so that Y is the max of R, G, B
		Y = std::max(R, std::max(G, B));

		double x, y;
		XYZ_to_xy(rawX, rawY, rawZ, x, y);
		X = x * Y / y;
		Z = (1 - x - y) * Y / y;
	}

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

	constexpr double PI = 3.14159265359;
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
#define exp2 _huemath::_exp2

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

    inline double _exp2(double a)
    {
        return pow(2, a);
    }
}
#endif
