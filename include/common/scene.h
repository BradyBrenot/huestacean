#pragma once

#include "device.h"
#include "common/math.h"
#include "effect.h"

#include <vector>
#include <tuple>
#include <string>
#include <memory>

struct DeviceInScene
{
	DevicePtr device;
	Math::Transform transform;

	DeviceInScene(DevicePtr& d, Math::Transform& t) :
		device(d), transform(t)
	{

	}

	DeviceInScene() :
		device(), transform()
	{

	}

	std::vector<Math::Box> GetLightBoundingBoxes() const;
};

struct Scene
{
	std::vector< DeviceInScene > devices;
	std::vector< std::unique_ptr<Effect> > effects;

	Scene() : devices(), effects() {}

	Scene(const Scene& r)
	{
		devices = r.devices;
		for (const auto& effect : r.effects)
		{
			effects.push_back(effect->clone());
		}
	}

	Scene(Scene&& r)
	{
		devices = std::move(r.devices);
		effects = std::move(r.effects);
	}

	Scene& operator=(const Scene& r)
	{
		devices = r.devices;
		for (const auto& effect : r.effects)
		{
			effects.push_back(effect->clone());
		}
		return *this;
	}
};

