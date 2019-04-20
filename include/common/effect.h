#pragma once

#include "common/math.h"

#include <vector>
#include <memory>

#include <QSettings>

class Effect
{
public:
	Effect() {}

	virtual void Tick(std::chrono::duration<float> deltaTime) {}
	virtual void Update(const std::vector<Math::Box>& positions, std::vector<Math::HsluvColor>& outColors) = 0;

	virtual void Save(QSettings& settings) = 0;
	static std::unique_ptr<Effect> StaticLoad(QSettings& settings);
	virtual void Load(QSettings& settings) = 0;

	std::unique_ptr<Effect> clone() const { return std::unique_ptr<Effect>(clone_impl()); }

protected:
	virtual Effect* clone_impl() const = 0;
};

class EffectWithTransform : public Effect
{
public:
	EffectWithTransform()
		: Effect(),
		transform()
	{

	}

	Math::Transform transform;
};