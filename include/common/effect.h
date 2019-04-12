#pragma once

#include "common/color.h"
#include "common/transform.h"

#include <vector>
#include <memory>

class Effect
{
public:
	virtual void Tick(float deltaTime) {}
	virtual void Update(const std::vector<Box>& positions, std::vector<HsluvColor>& outColors) = 0;


	std::unique_ptr<Effect> clone() const { return std::unique_ptr<Effect>(clone_impl()); }

protected:
	virtual Effect* clone_impl() const = 0;
};

class DerivedEffect : public Effect
{
public:
	virtual void Update(const std::vector<Box>& positions, std::vector<HsluvColor>& outColors) override {}

protected:
	virtual Effect* clone_impl() const override
	{
		return new DerivedEffect(*this);
	}
};