#pragma once

#include "device.h"
#include "transform.h"

struct DeviceInRoom
{
	Transform transform;
	std::unique_ptr<Device> device;

	String Serialize();
	static DeviceInRoom Deserialize(String);
};

class Room
{
public:

	void Tick(float DeltaTime);

	void AddDevice(std::unique_ptr<Device> device, Transform transform);
	void MoveDevice(std::unique_ptr<Device> device, Transform transform);
	void RemoveDevice(std::unique_ptr<Device> device);

	void AddEffect(std::unique_ptr<Effect> effect);
	void ModifyEffect(std::unique_ptr<Effect> effect);
	std::vector<Effect>[] GetEffects();

private:
	std::mutex lock;
	std::vector< DeviceInRoom > devices;
	std::vector< Effect > effects;
};

