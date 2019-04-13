#include "common/math.h"

#include <cmath>

using namespace Math;

void XYZ_to_LCh(double& X, double& Y, double& Z, double& L, double& C, double& h)
{
	//Intermediate coordinates (a* and b* of L*a*b*)
	double a, b;

	auto f = [&](double t) -> double {
		constexpr double d = 6.0 / 29.0;
		constexpr double dcubed = d*d*d;

		if (t > dcubed)	{
			return std::cbrt(t);
		}
		
		return (t / (3.0 * std::pow(d, 2))) + (4.0 / 29.0);
	};

	L = 116.0 * f(Y / D65_Yn) - 16.0;
	a = 500.0 * (f(X / D65_Xn) - f(Y / D65_Yn));
	b = 200.0 * (f(Y / D65_Yn) - f(Z / D65_Zn));

	C = sqrt(pow(a, 2) + pow(b, 2));
	h = atan2(b, a);
}

void LCh_to_XYZ(double& L, double& C, double& h, double& X, double& Y, double& Z)
{
	//Intermediate coordinates (a* and b* of L*a*b*)
	double a, b;

	a = C * cos(h);
	b = C * sin(h);

	auto f = [&](double t) -> double {
		constexpr double d = 6.0 / 29.0;
		if (t > d) {
			return pow(t, 3);
		}

		return 3 * pow(d, 2) * (t - 4.0 / 29.0);
	};

	auto c = [&]() {
		X = D65_Xn * f((L + 16.0) / 116.0 + a / 500.0);
		Y = D65_Yn * f((L + 16.0) / 116.0);
		Z = D65_Zn * f((L + 16.0) / 116.0 - b / 200.0);
	};

	c();

	//stupid method to brute force us out of bad C values
	if (X + Y + Z < 0)
	{
		double oa = a;
		double ob = b;

		for (double i = 1.2; i < 3 && (X + Y + Z < 0); i += 0.2)
		{
			a = oa / (i);
			b = ob / (i);
			c();
		}
	}

#if 0
	double fY = (L + 16.0) / 116.0;
	double fX = a / 500.0 + fY;
	double fZ = fY - b / 200.0;

	double e = 0.008856;
	double k = 903.3;

	X = D65_Xn * [&]() -> double {
		double fX3 = std::pow(fX, 3);
		//return fX3 > e ? fX3 : (116.0 * fX - 16.0) / k;
		return fX3 > e ? fX3 : (fX - 16.0 / 116.0) / 7.787;
	}();

	Y = D65_Yn * [&]() -> double {
		return L > k*e ? std::pow(((L + 16.0) / 116.0), 3) : L / k;
	}();

	Z = D65_Zn * [&]() -> double {
		double fZ3 = std::pow(fZ, 3);
		return fZ3 > e ? fZ3 : (fZ - 16.0 / 116.0) / 7.787;
	}();
#endif

#if 0
	//Suppress yellow, the LED is too bright
	//0.35 - 2.5
	//0.6 to 1.7, 1.15 mid
	double yellow = 1.7;
	double yellowWidth = 1.1;

	double distFromYellow = std::abs(yellow - h);
	if (distFromYellow < yellowWidth)
	{
		double suppression = 1.4 * (1.0 - (distFromYellow / yellowWidth));
		suppression = std::min(1.0, suppression);
		Y = std::min(L, Utility::lerp(Y, Y * 0.8, suppression));
		qDebug() << distFromYellow << h << suppression;
	}
#endif

	//0.446613 0.384795
	//0.400317 0.359206
}

bool InGamut(Vector2d p)
{
	//gamut C
	Vector2d red(0.692, 0.308);
	Vector2d green(0.17, 0.7);
	Vector2d blue(0.153, 0.048);

	Vector2d v1 = Vector2d(green.x - red.x, green.y - red.y);
	Vector2d v2 = Vector2d(blue.x - red.x, blue.y - red.y);

	Vector2d q = Vector2d(p.x - red.x, p.y - red.y);

	double s = q.Cross(v2) / v1.Cross(v2);
	double t = v1.Cross(q) / v1.Cross(v2);

	if ((s >= 0.0f) && (t >= 0.0f) && (s + t <= 1.0f)) {
		return true;
	}
	else {
		return false;
	}
}

Vector2d getClosestPointToPoints(Vector2d A, Vector2d B, Vector2d P) {
	Vector2d AP(P.x - A.x, P.y - A.y);
	Vector2d AB(B.x - A.x, B.y - A.y);
	double ab2 = AB.x * AB.x + AB.y * AB.y;
	double ap_ab = AP.x * AB.x + AP.y * AB.y;

	double t = ap_ab / ab2;

	if (t < 0.0f) {
		t = 0.0f;
	}
	else if (t > 1.0f) {
		t = 1.0f;
	}

	Vector2d newPoint(A.x + AB.x * t, A.y + AB.y * t);
	return newPoint;
}

void FitInGamut(double &x, double& y)
{
	//This doesn't actually fit it in the gamut, it just keeps it in (0,0) to (1,1)
	//The lights are expected to correct this themselves, which they seem to do just fine

	if (x < 0 || x > 1.0 || y < 0 || y > 1.0)
	{
		double diffX = (x - D65_x);
		double diffY = (y - D65_y);
		double unitX = diffX == 0.0 ? 0.0 : diffX / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));
		double unitY = diffY == 0.0 ? 0.0 : diffY / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));

		double bestDist = 0;
		double testDist;

		if (unitX > 0.0)
		{
			testDist = (1.0 - D65_x) / unitX;
			if (D65_y + testDist * unitY <= 1.0 && D65_y + testDist * unitY >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}
		else
		{
			testDist = (-D65_x) / unitX;
			if (D65_y + testDist * unitY <= 1.0 && D65_y + testDist * unitY >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}

		if (unitY > 0.0)
		{
			testDist = (1.0 - D65_y) / unitY;
			if (D65_x + testDist * unitX <= 1.0 && D65_x + testDist * unitX >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}
		else
		{
			testDist = (-D65_y) / unitY;
			if (D65_x + testDist * unitX <= 1.0 && D65_x + testDist * unitX >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}

		x = D65_x + unitX * bestDist;
		y = D65_y + unitY * bestDist;
	}
}

void XYZ_to_xy(double& X, double& Y, double& Z, double& x, double& y)
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
	if constexpr (doGammaCorrection)
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

void rgb_to_xy(double& r, double& g, double& b, double& x, double& y, double& Y)
{
	double X, Z;

	rgb_to_XYZ<true>(r, g, b, X, Y, Z);
	XYZ_to_xy(X, Y, Z, x, y);
}