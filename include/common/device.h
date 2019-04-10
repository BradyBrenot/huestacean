#pragma once
#include "transform.h"

#include <tuple>
#include <string>
#include <memory>

struct Device
{
	explicit Device(std::vector<Box>&& inLights);
	Device();

	virtual std::tuple<std::unique_ptr<class DeviceData>, ProviderType> getLightData(Transform transform) = 0;
	std::string serialize();

	virtual bool operator==(const std::unique_ptr<Device>& other) const = 0;
	bool operator!=(const std::unique_ptr<Device>& other) const
	{
		return !(*this == other);
	}

protected:
	std::vector<Box> lights;
	std::vector<Box> transformLights(Transform transform);
};