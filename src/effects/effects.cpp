#include "effects/effects.h"

#include <cmath>

using namespace std::chrono_literals;
using namespace Math;

SinePulseEffect::SinePulseEffect()
	: counter(0.0)
{

}

SinePulseEffect::SinePulseEffect(const SinePulseEffect& s)
	: counter(s.counter)
{

}

void SinePulseEffect::Tick(std::chrono::duration<float> deltaTime)
{
	counter += deltaTime;
}

void SinePulseEffect::Update(const std::vector<Math::Box>& positions, std::vector<Math::HsluvColor>& outColors)
{
	constexpr auto hRate = 1.0;
	constexpr auto sRate = 2.0;
	constexpr auto lRate = 3.0;

	constexpr auto hMax = PI;
	constexpr auto sMax = 100.0;
	constexpr auto lMax = 100.0;

	constexpr auto hMult = 0.5 * hMax;
	constexpr auto sMult = 0.5 * sMax;
	constexpr auto lMult = 0.5 * lMax;

	auto Color = Math::HsluvColor(
		std::sin(counter.count() * hRate) * hMult,
		std::sin(counter.count() * sRate) * sMult,
		std::sin(counter.count() * lRate) * lMult);

	for (auto c : outColors)
	{
		c = Color;
	}
}