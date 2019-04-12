#pragma once
#include "transform.h"
#include "providertype.h"
#include <vector>
#include <string>

class Device
{
	Device() = delete;
	explicit Device(ProviderType inType);

	ProviderType type;
	std::string Serialize() const;

	virtual std::vector<Box> GetLightLocations() const = 0;

protected:
	virtual std::string Serialize_Internal() = 0;
};