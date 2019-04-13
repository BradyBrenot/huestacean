#pragma once

#include <cmath>
#include <algorithm>

namespace Math
{
	// It's one "big" namespace
	// but it's not very big, I don't want to split it up unnecessarily just to pretend it's tidier

	//////////////////////
	// Units
	//////////////////////
	typedef double distance;
	typedef double angle;

	//////////////////////
	// Constants
	//////////////////////

	// CIE 1931 coordinates of CIE Standard Illuminant D65
	constexpr double D65_x = 0.31271;
	constexpr double D65_y = 0.32902;

	// Normalized XYZ coordinates of D65 (normalized s.t. Y = 100)
	constexpr double D65_Xn = 95.047;
	constexpr double D65_Yn = 100.0;
	constexpr double D65_Zn = 108.883;

	constexpr double PI = 3.14159265359;

	//////////////////////
	// Literals
	//////////////////////

	inline distance operator""_m(long double d) {
		return distance( d );
	}

	inline distance operator""_cm(long double d) {
		return distance( d / 100.0 );
	}

	inline angle operator""_rad(long double a) {
		return angle( a );
	}

	inline angle operator""_deg(long double a) {
		return angle( a * PI / 180.0 );
	}

	inline distance operator""_m(unsigned long long d) {
		return distance(d);
	}

	inline distance operator""_cm(unsigned long long d) {
		return distance(distance(d) / 100.0);
	}

	inline angle operator""_rad(unsigned long long a) {
		return angle(angle(a));
	}

	inline angle operator""_deg(unsigned long long a) {
		return angle(long double(a) * PI / 180.0);
	}

	//////////////////////
	// Complex types
	//////////////////////


	// Colors
	struct RgbColor;
	struct HsluvColor;
	struct XyzColor;

	struct HsluvColor
	{
		double h;
		double s;
		double l;

		HsluvColor();
		HsluvColor(const RgbColor& from);
	};

	struct RgbColor
	{
		double r;
		double g;
		double b;

		RgbColor();
		RgbColor(const HsluvColor& from);
	};

	struct XyzColor
	{
		double x;
		double y;
		double z;

		XyzColor();
		XyzColor(const RgbColor& from);
		XyzColor(const HsluvColor& from);
	};

	// Coordinate / angle types

	struct Vector3d
	{
		double x, y, z;

		Vector3d()
			: x(0), y(0), z(0)
		{
		}
		Vector3d(double inX, double inY, double inZ)
			: x(inX), y(inY), z(inZ)
		{
		}

	};

	struct Vector2d {
		double x, y;

		constexpr Vector2d()
			: x(0), y(0)
		{
		}
		constexpr Vector2d(double inX, double inY)
			: x(inX), y(inY)
		{
		}

		double Size()
		{
			return std::sqrt(x * x + y * y);
		}

		constexpr double Cross(Vector2d v2)
		{
			return (x * v2.y - y * v2.x);
		}

		constexpr Vector2d operator-(const Vector2d& b) const
		{
			return Vector2d(x - b.x, y - b.y);
		}
		constexpr Vector2d operator+(const Vector2d& b) const
		{
			return Vector2d(x + b.x, y + b.y);
		}
	};


	//NB: gimbal lock. Doesn't matter for Huestacean presently
	struct Rotator
	{
		angle pitch, yaw, roll;

		Rotator()
			: pitch(0_rad), yaw(0_rad), roll(0_rad)
		{
		}
		Rotator(angle inPitch, angle inYaw, angle inRoll)
			: pitch(inPitch), yaw(inYaw), roll(inRoll)
		{
		}
	};

	struct Box
	{
		Vector3d center;
		Vector3d halfSize;

		Box()
			: center(), halfSize()
		{
		}

		Box(Vector3d inCenter, Vector3d inHalfSize)
			: center(inCenter), halfSize(inHalfSize)
		{
		}
	};

	struct Transform
	{
		Vector3d location;
		Vector3d scale;
		Rotator rotation;

		Box transformBox(const Box& b) const;
	};
};

namespace Math
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
