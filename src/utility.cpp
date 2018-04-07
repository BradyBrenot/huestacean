#include "utility.h"

#include <algorithm>
#include <cmath>

void Color::XYZ_to_LCh(double& X, double& Y, double& Z, double& L, double& C, double& h)
{
	//Intermediate coordinates (a* and b* of L*a*b*)
	double a, b;

	auto f = [&](double t) -> double {
		constexpr double δ = 6.0 / 29.0;
		constexpr double δcubed = δ*δ*δ;

		if (t > δcubed)	{
			return cbrt(t);
		}
		
		return (t / (3.0 * pow(δ, 2))) + (4.0 / 29.0);
	};

	L = 116.0 * f(Y / D65_Yn) - 16.0;
	a = 500.0 * (f(X / D65_Xn) - f(Y / D65_Yn));
	b = 200.0 * (f(Y / D65_Yn) - f(Z / D65_Zn));

	C = sqrt(pow(a, 2) + pow(b, 2));
	h = atan2(b, a);
}

void Color::LCh_to_XYZ(double& L, double& C, double& h, double& X, double& Y, double& Z)
{
	//Intermediate coordinates (a* and b* of L*a*b*)
	double a, b;

	auto f = [&](double t) -> double {
		constexpr double δ = 6.0 / 29.0;
		if (t > δ) {
			return pow(t, 3);
		}

		return 3 * pow(δ, 2) * (t - 4.0 / 29.0);
	};

	a = C * cos(h);
	b = C * sin(h);

	X = D65_Xn * f((L + 16.0) / 116.0 + a / 500.0);
	Y = D65_Yn * f((L + 16.0) / 116.0);
	Z = D65_Zn * f((L + 16.0) / 116.0 - b / 200.0);
}

void Color::XYZ_to_xy(double& X, double& Y, double& Z, double& x, double& y)
{
	if (X + Y + Z == 0)
	{
		x = D65_x;
		y = D65_y;
	}
	else
	{
		x = X / (X + Y + Z);
		y = Y / (X + Y + Z);
	}
}

void Color::rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& Y)
{
	double X, Z;

	rgb_to_XYZ<true>(r, g, b, X, Y, Z);
	XYZ_to_xy(X, Y, Z, x, y);
}