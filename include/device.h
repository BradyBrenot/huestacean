#pragma once
#include "transform.h"

#include <tuple>

struct Device
{
	explicit Device(std::vector<Box>&& inLights);
	Device();

	virtual std::tuple<std::unique_ptr<class DeviceData>, ProviderType> getLightData(Transform transform) = 0;
	string serialize();

	virtual bool operator==(const unique_ptr<Device>& other) = 0;
	bool operator!=(const unique_ptr<Device>& other)
	{
		return !(this == other);
	}

protected:
	std::vector<Box> lights;
	std::vector<Box> transformLights(Transform transform);
};