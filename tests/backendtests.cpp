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

		auto sr = b.GetWriter();
		{
			auto& scenes = sr.GetScenesMutable();

			Scene s;
			s.devices.push_back(DeviceInScene{ light, t });
		}
		

		sr.Save();
		sr.Load();
	}
	
}

TEST_CASE("Backend can save and load a scene and it turns out OK", "[save]") {
	QSettings s;
	s.clear();

	auto light = std::make_shared<Hue::Light>();
	DevicePtr lightAsDevice = DevicePtr{ light };
	light->name = "dummy";
	Transform t;
	t.location.x = 42;

	{
		Backend b;

		auto sr = b.GetWriter();
		{
			auto& scenes = sr.GetScenesMutable();

			Scene s;
			s.devices.push_back(DeviceInScene{ lightAsDevice, t });
			scenes.push_back(s);
		}

		sr.Save();
	}


	{
		Backend b;
		auto sr = b.GetWriter();

		sr.Load();
		auto scenes = sr.GetScenes();
		REQUIRE(scenes.size() == 1);
		REQUIRE(scenes[0].devices.size() == 1);

		REQUIRE(scenes[0].devices[0].transform.location.x == t.location.x);
	}
}