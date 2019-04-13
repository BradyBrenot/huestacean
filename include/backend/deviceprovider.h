#pragma once
#include "common/lightupdate.h"
#include "common/device.h"
#include "common/room.h"

#include <functional>

class DeviceProvider
{
public:
	ProviderType GetType() const {
		return type;
	}

	virtual void Update(const LightUpdateParams& Params) = 0;
	virtual std::vector<DevicePtr> GetDevices() = 0;

	virtual bool compare(DeviceInRoom a, DeviceInRoom b)
	{
		return ::compare(a.device, b.device);
	};

protected:
	DeviceProvider() = delete;
	explicit DeviceProvider(ProviderType inType) : type(inType)
	{

	}

private:
	ProviderType type;
};

namespace std
{
	template <>
	struct hash<DeviceProvider>
	{
		std::size_t operator()(const DeviceProvider& p) const
		{
			return hash<ProviderType>()(p.GetType());
		}
	};
}