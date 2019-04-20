#pragma once

#include "common/effect.h"

//////////////////////////////////////////////////////////////////////////
//
// Some simple effects
//
//////////////////////////////////////////////////////////////////////////

class SinePulseEffect : public EffectWithTransform
{
public:
	SinePulseEffect();
	SinePulseEffect(const SinePulseEffect& s);

	virtual void Tick(std::chrono::duration<float> deltaTime) override;
	virtual void Update(const std::vector<Math::Box>& positions, std::vector<Math::HsluvColor>& outColors) override;
	virtual void Save(QSettings& settings) override {}
	virtual void Load(QSettings& settings) override {}

protected:
	virtual Effect* clone_impl() const override { return new SinePulseEffect(*this); }

private:
	std::chrono::duration<float> counter;
};