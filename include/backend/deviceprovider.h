#pragma once
#include "common/lightupdate.h"
#include "common/device.h"

class DeviceProvider
{
	DeviceProvider() = delete;
	explicit DeviceProvider(ProviderType inType);

	ProviderType GetType() {
		return type;
	}

	virtual void Update(const LightUpdateParams& Params) = 0;
	virtual std::vector<DevicePtr> GetDevices() = 0;
	virtual bool compare(DevicePtr a, DevicePtr b)
	{
		return ::compare(a, b);
	};

protected:
	ProviderType type;
};