#include "catch/catch.hpp"
#include "backend/backend.h"
#include "hue/hue.h"

#include <chrono>
#include <thread>
#include "common/math.h"

#include <QSettings>

using namespace std::chrono_literals;
using namespace Math;

TEST_CASE("can start and stop Backend", "") {
	Backend b;

	b.Start();
	REQUIRE(b.IsRunning());

	b.Stop();
	REQUIRE(!b.IsRunning());
}

TEST_CASE("Backend lets me save and load", "") {
	DevicePtr light = std::make_shared<Hue::Light>();
	Transform t;
	

	{
		Backend b;

		auto sr = b.GetScenesWriter();
		{
			auto& scenes = sr.GetScenesMutable();

			Scene s;
			s.devices.push_back(DeviceInScene{ light, t });
		}
		

		b.Save();
		b.Load();
	}
	
}

TEST_CASE("Backend can save and load a scene and it turns out OK", "") {
	QSettings s;
	s.clear();

	auto light = std::make_shared<Hue::Light>();
	DevicePtr lightAsDevice = DevicePtr{ light };
	light->name = "dummy";
	Transform t;
	t.location.x = 42;

	{
		Backend b;

		auto sr = b.GetScenesWriter();
		{
			auto& scenes = sr.GetScenesMutable();

			Scene s;
			s.devices.push_back(DeviceInScene{ lightAsDevice, t });
		}

		b.Save();
	}


	{
		Backend b;

		b.Load();
		auto scenes = b.GetScenes();
		REQUIRE(scenes.size() == 1);
		REQUIRE(scenes[0].devices.size() == 1);
		
		auto loadedLight = std::dynamic_pointer_cast<Hue::Light>(scenes[0].devices[0].device);
		REQUIRE(loadedLight != nullptr);
		REQUIRE(loadedLight->name == light->name);
		REQUIRE(scenes[0].devices[0].transform.location.x == t.location.x);
	}
}