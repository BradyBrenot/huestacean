#include "catch/catch.hpp"

#include "common/math.h"
#include "thirdparty/hsluv-c/tests/snapshot.h"
#include <cmath>

using namespace Math;


constexpr double EPSILON = 0.00000001;

auto nearlyEqual(double a, double b)
{
	return std::abs(a - b) < EPSILON;
}

//Just copying hsluv-c's tests, make sure wrapper code isn't screwing anything up
TEST_CASE("Rgb => Hsluv", "") {
	for (int i = 0; i < snapshot_n; i++) {
		auto hsl = HsluvColor{ snapshot[i].hsluv_h, snapshot[i].hsluv_s, snapshot[i].hsluv_l };
		auto rgb = RgbColor{ hsl };

		REQUIRE(nearlyEqual(rgb.r, snapshot[i].rgb_r));
		REQUIRE(nearlyEqual(rgb.g, snapshot[i].rgb_g));
		REQUIRE(nearlyEqual(rgb.b, snapshot[i].rgb_b));
	}
}

TEST_CASE("Hsluv => RGB", "") {
	for (int i = 0; i < snapshot_n; i++) {
		auto rgb = RgbColor{ snapshot[i].rgb_r, snapshot[i].rgb_g, snapshot[i].rgb_b };
		auto hsl = HsluvColor{ rgb };

		REQUIRE(nearlyEqual(hsl.h, snapshot[i].hsluv_h));
		REQUIRE(nearlyEqual(hsl.s, snapshot[i].hsluv_s));
		REQUIRE(nearlyEqual(hsl.l, snapshot[i].hsluv_l));
	}
}

struct TransformTestCase
{
	Box start;
	Box goal;
	Transform transform;
};

TEST_CASE("Transform Box", "") {
	auto vectorNearlyEqual = [&](const Vector3d & a, const Vector3d & b) {
		return nearlyEqual(a.x, b.x) && nearlyEqual(a.y, b.y) && nearlyEqual(a.z, b.z);
	};

	auto boxNearlyEqual = [&](const Box & a, const Box & b)	{
		return vectorNearlyEqual(a.center, b.center) &&	vectorNearlyEqual(a.halfSize, b.halfSize);
	};

	auto testCases = {
		TransformTestCase{ {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}, { {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}} }
	};

	for (const auto& test: testCases)
	{
		auto transformed = test.transform.transformBox(test.start);
		REQUIRE(boxNearlyEqual(transformed, test.goal));
	}
}