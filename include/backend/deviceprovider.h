#pragma once
#include "common/lightupdate.h"
#include "common/device.h"
#include "common/scene.h"

#include <functional>

class DeviceProvider
{
public:
	ProviderType GetType() const {
		return type;
	}

	virtual void Start() {}
	virtual void Stop() {}
	virtual void UpdateThreadCleanup() {}

	virtual void Update(const LightUpdateParams& Params) = 0;
	virtual std::vector<DevicePtr> GetDevices() const = 0;

	virtual bool compare(DeviceInScene a, DeviceInScene b)
	{
		return ::compare(a.device, b.device);
	};

	virtual DevicePtr GetDeviceFromUniqueId(std::string id) const = 0;

	virtual void Save(QSettings& settings) {};
	virtual void Load(QSettings& settings) {};

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