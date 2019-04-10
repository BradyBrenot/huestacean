#pragma once

#include "common/color.h"

#include <vector>
#include <tuple>
#include <string>
#include <memory>

class Effect
{
	virtual void Tick(float deltaTime) {}
	virtual void Update(const std::vector<Box>& positions, std::vector<HsluvColor>& outColors) = 0;

	virtual bool operator==(const Effect& other) = 0;
	bool operator!=(const Effect& other)
	{
		return !(*this == other);
	}
};