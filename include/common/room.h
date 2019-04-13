#pragma once

#include "device.h"
#include "common/math.h"
#include "effect.h"

#include <vector>
#include <tuple>
#include <string>
#include <memory>

struct DeviceInRoom
{
	Math::Transform transform;
	DevicePtr device;
	std::vector<Math::Box> lightLocations;
};

struct Room
{
	std::vector< DeviceInRoom > devices;
	std::vector< std::unique_ptr<Effect> > effects;

	Room() : devices(), effects() {}

	Room(const Room& r)
	{
		devices = r.devices;
		for (const auto& effect : r.effects)
		{
			effects.push_back(effect->clone());
		}
	}

	Room(Room&& r)
	{
		devices = std::move(r.devices);
		effects = std::move(r.effects);
	}

	Room& operator=(const Room& r)
	{
		devices = r.devices;
		for (const auto& effect : r.effects)
		{
			effects.push_back(effect->clone());
		}
		return *this;
	}
};

