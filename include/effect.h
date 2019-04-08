#pragma once

class Effect
{
	virtual void Tick(float deltaTime) {}
	virtual void Update(const std::vector<Box>& positions, std::vector<HsluvColor>& outColors) = 0;

	virtual bool operator==(const unique_ptr<Effect>& other) = 0;
	bool operator!=(const unique_ptr<Effect>& other)
	{
		return !(this == other);
	}
};