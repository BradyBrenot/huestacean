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
		device(d), 
		transform(t)
	{

	}

	DeviceInScene() :
		device(), 
		transform{ {0, 0, 0}, {1.0, 1.0, 1.0}, {0, 0, 0} }
	{

	}

	std::vector<Math::Box> GetLightBoundingBoxes() const;
};

struct Scene
{
	std::string name;
	std::vector< DeviceInScene > devices;
	std::vector< std::unique_ptr<Effect> > effects;
	Math::Vector3d size;

	Scene() : 
		name(),
		devices(), 
		effects(),
		size(10, 10, 10)
	{
	
	}

	Scene(const Scene& r)
	{
		*this = r;
	}

	Scene(Scene&& r)
	{
		name = std::move(r.name);
		devices = std::move(r.devices);
		effects = std::move(r.effects);
		size = r.size;
	}

	Scene& operator=(const Scene& r)
	{
		name = r.name;
		devices = r.devices;
		effects.clear();
		for (const auto& effect : r.effects)
		{
			effects.push_back(effect->clone());
		}
		size = r.size;
		return *this;
	}
};

