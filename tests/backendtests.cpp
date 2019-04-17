#include "catch/catch.hpp"
#include "backend/backend.h"
#include "hue/hue.h"

#include <chrono>
#include <thread>
#include "common/math.h"

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