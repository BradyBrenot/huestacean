#pragma once

#include "common/math.h"

#include <vector>
#include <memory>

class Effect
{
public:
	virtual void Tick(float deltaTime) {}
	virtual void Update(const std::vector<Math::Box>& positions, std::vector<Math::HsluvColor>& outColors) = 0;


	std::unique_ptr<Effect> clone() const { return std::unique_ptr<Effect>(clone_impl()); }

protected:
	virtual Effect* clone_impl() const = 0;
};

class DerivedEffect : public Effect
{
public:
	virtual void Update(const std::vector<Math::Box>& positions, std::vector<Math::HsluvColor>& outColors) override {}

protected:
	virtual Effect* clone_impl() const override
	{
		return new DerivedEffect(*this);
	}
};