#pragma once

#include "device.h"
#include "transform.h"

struct DeviceInRoom
{
	Transform transform;
	std::unique_ptr<Device> device;
};

class Room
{
	std::mutex lock;

	void AddDevice(std::unique_ptr<Device> device, Transform transform);
	void MoveDevice(std::unique_ptr<Device> device, Transform transform);
	void RemoveDevice(std::unique_ptr<Device> device);

	void AddEffect(std::unique_ptr<Effect> effect);

};

