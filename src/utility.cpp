#include "utility.h"

void Color::XYZ_to_LCh(double& X, double& Y, double& Z, double& L, double& C, double& h)
{
	//Intermediate coordinates (a* and b* of L*a*b*)
	double a, b;

	auto f = [&](double t) -> double {
		constexpr double d = 6.0 / 29.0;
		constexpr double dcubed = d*d*d;

		if (t > dcubed)	{
			return cbrt(t);
		}
		
		return (t / (3.0 * pow(d, 2))) + (4.0 / 29.0);
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

struct vector2d {
	double x;
	double y;

	vector2d(double inx, double iny) : x(inx), y(iny) {

	}
};

float dist(vector2d v1, vector2d v2)
{
	float dx = v1.x - v2.x;
	float dy = v1.y - v2.y;
	float dist = sqrt(dx * dx + dy * dy);

	return dist;
}

float cross(vector2d v1, vector2d v2)
{
	return (v1.x * v2.y - v1.y * v2.x);
}

bool InGamut(vector2d p)
{
	//gamut C
	vector2d red(0.692, 0.308);
	vector2d green(0.17, 0.7);
	vector2d blue(0.153, 0.048);

	vector2d v1 = vector2d(green.x - red.x, green.y - red.y);
	vector2d v2 = vector2d(blue.x - red.x, blue.y - red.y);

	vector2d q = vector2d(p.x - red.x, p.y - red.y);

	float s = cross(q, v2) / cross(v1, v2);
	float t = cross(v1, q) / cross(v1, v2);

	if ((s >= 0.0f) && (t >= 0.0f) && (s + t <= 1.0f)) {
		return true;
	}
	else {
		return false;
	}
}

vector2d getClosestPointToPoints(vector2d A, vector2d B, vector2d P) {
	vector2d AP(P.x - A.x, P.y - A.y);
	vector2d AB(B.x - A.x, B.y - A.y);
	float ab2 = AB.x * AB.x + AB.y * AB.y;
	float ap_ab = AP.x * AB.x + AP.y * AB.y;

	float t = ap_ab / ab2;

	if (t < 0.0f) {
		t = 0.0f;
	}
	else if (t > 1.0f) {
		t = 1.0f;
	}

	vector2d newPoint(A.x + AB.x * t, A.y + AB.y * t);
	return newPoint;
}

void Color::FitInGamut(double &x, double& y)
{
	//This doesn't actually fit it in the gamut, it just keeps it in (0,0) to (1,1)
	//The lights are expected to correct this themselves, which they seem to do just fine

	if (x < 0 || x > 1.0 || y < 0 || y > 1.0)
	{
		double diffX = (x - Color::D65_x);
		double diffY = (y - Color::D65_y);
		double unitX = diffX == 0.0 ? 0.0 : diffX / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));
		double unitY = diffY == 0.0 ? 0.0 : diffY / std::sqrt(std::pow(diffX, 2.0) + std::pow(diffY, 2.0));

		double bestDist = 0;
		double testDist;

		if (unitX > 0.0)
		{
			testDist = (1.0 - Color::D65_x) / unitX;
			if (Color::D65_y + testDist * unitY <= 1.0 && Color::D65_y + testDist * unitY >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}
		else
		{
			testDist = (-Color::D65_x) / unitX;
			if (Color::D65_y + testDist * unitY <= 1.0 && Color::D65_y + testDist * unitY >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}

		if (unitY > 0.0)
		{
			testDist = (1.0 - Color::D65_y) / unitY;
			if (Color::D65_x + testDist * unitX <= 1.0 && Color::D65_x + testDist * unitX >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}
		else
		{
			testDist = (-Color::D65_y) / unitY;
			if (Color::D65_x + testDist * unitX <= 1.0 && Color::D65_x + testDist * unitX >= 0.0)
			{
				bestDist = std::max(bestDist, testDist);
			}
		}

		x = Color::D65_x + unitX * bestDist;
		y = Color::D65_y + unitY * bestDist;
	}

#if 0
	//gamut C
	vector2d red(0.692, 0.308);
	vector2d green(0.17, 0.7);
	vector2d blue(0.153, 0.048);

	vector2d v(x, y);

	bool inGamut = InGamut(v);
	if (!inGamut)
	{
		//fit to gamut
		// draw a line between (x,y) and D65
		// find intersects between this line and the edges of the gamut triangle
		// select closest intersect


		//philips' recommended method of snapping to gamut. It snaps to the corners too readily

		//Find the closest point on each line in the triangle.
		vector2d pAB = getClosestPointToPoints(red, green, v);
		vector2d pAC = getClosestPointToPoints(blue, red, v);
		vector2d pBC = getClosestPointToPoints(green, blue, v);

		//Get the distances per point and see which point is closer to our Point.
		float dAB = dist(v, pAB);
		float dAC = dist(v, pAC);
		float dBC = dist(v, pBC);

		float lowest = dAB;
		vector2d closestPoint = pAB;

		if (dAC < lowest) {
			lowest = dAC;
			closestPoint = pAC;
		}
		if (dBC < lowest) {
			lowest = dBC;
			closestPoint = pBC;
		}

		//Change the xy value to a value which is within the reach of the lamp.
		x = closestPoint.x;
		y = closestPoint.y;
	}
#endif
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