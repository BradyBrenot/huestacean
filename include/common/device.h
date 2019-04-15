#pragma once
#include "common/math.h"
#include "providertype.h"
#include <vector>
#include <string>
#include <memory>
#include <atomic>

// NB: all public functions must enforce thread-safety

class Device
{
public:

	ProviderType GetType() const {
		return type;
	}

	bool IsConnected() const {
		return isConnected;
	}

	void setIsConnected(bool in) {
		isConnected = in;
	}

	virtual std::vector<Math::Box> GetLightBoundingBoxes() const = 0;

protected:
	Device() = delete;
	explicit Device(ProviderType inType)
		: type(inType), 
		isConnected(false)
	{

	}

private:
	ProviderType type;
	std::atomic_bool isConnected;
};

//I don't know if Device should be mutable
typedef std::shared_ptr<Device> DevicePtr;

inline bool compare(const DevicePtr& a, const DevicePtr& b) {
	return a->GetType() > b->GetType();
}