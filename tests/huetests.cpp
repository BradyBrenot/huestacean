#include "catch/catch.hpp"
#include "backend/backend.h"
#include "hue/hue.h"
#include "hue/bridge.h"
#include "effects/effects.h"

#include <iostream>

#include <memory>
#include <QTest>

TEST_CASE("a Backend has a Hue device provider", "[.][hue][hueAll]") {
	Backend b;

	auto* hue = b.GetDeviceProvider(ProviderType::Hue);
	REQUIRE(hue != nullptr);

	SECTION("Hue's ProviderType is right") {
		REQUIRE(hue->GetType() == ProviderType::Hue);
	}
}

TEST_CASE("the Hue device provider can connect with bridges", "[.][hueAll]") {

	SECTION("Finds a bridge from scratch, links it, and finds devices on it") {
		Backend b;

		auto& hue = b.hue;

		hue.SearchForBridges(std::vector<std::string>(), true);

		REQUIRE(QTest::qWaitFor([&]() { return hue.GetBridges().size() > 0; }, 5000));

		auto& bridges = hue.GetBridges();
		bridges[0]->Connect();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->GetStatus() == Hue::Bridge::Status::Connected; }, 10000));

		bridges[0]->RefreshDevices();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->devices.size() > 0; }, 2000));

		auto sr = b.GetWriter();
		sr.Save();
	}
}

TEST_CASE("Connects to a bridge from a previous test from a previous test", "[.][hue][hueAll]") {
	Backend b;
	auto sr = b.GetWriter();
	sr.Load();

	auto& hue = b.hue;

	hue.SearchForBridges(std::vector<std::string>(), true);

	REQUIRE(QTest::qWaitFor([&]() { return hue.GetBridges().size() > 0; }, 5000));

	auto& bridges = hue.GetBridges();
	bridges[0]->Connect();

	REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->GetStatus() == Hue::Bridge::Status::Connected; }, 10000));
}

TEST_CASE("Lights can animate", "[.][hueAll][prompt]") {
	Backend b;

	{
		auto sr = b.GetWriter();
		sr.Load();
	}

	{
		auto& hue = b.hue;

		hue.SearchForBridges(std::vector<std::string>(), true);

		REQUIRE(QTest::qWaitFor([&]() { return hue.GetBridges().size() > 0; }, 5000));

		auto& bridges = hue.GetBridges();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->GetStatus() == Hue::Bridge::Status::Connected; }, 10000));
		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->devices.size() > 0; }, 2000));

		auto sr = b.GetWriter();
		auto& scenes = sr.GetScenesMutable();

		int totalDevices = 0;
		Scene s;
		for (auto& b : bridges)
		{
			for (auto& d : b->devices)
			{
				++totalDevices;
			}
		}

		int i = 0;
		for (auto& b : bridges)
		{
			for (auto& d : b->devices)
			{
				DeviceInScene& dis = s.devices.emplace_back();
				dis.device = d;
				dis.transform.location.x = s.size.x * (i / totalDevices);
				dis.transform.location.y = s.size.y * (i / totalDevices);
				dis.transform.location.z = s.size.z * (i / totalDevices);
			}
		}

		s.effects.push_back(std::make_unique<SinePulseEffect>());
		
		scenes.clear();
		scenes.push_back(s);
	}

	b.SetActiveScene(0);
	
	b.Start();

	std::string answer;
	std::atomic_bool answered = false;

	auto thread = std::thread([&] {
		std::cout << "Are the lights (at most 10 per bridge) animating? [Y/N]: ";
		std::cin >> answer;
		answered = true;
	});

	while (!answered) {
		QTest::qWait(100);
	}

	thread.join();

	{
		auto sr = b.GetWriter();
		sr.Save();
	}

	bool saidYes = answer.rfind('y', 0) == 0 || answer.rfind('Y', 0) == 0;
	REQUIRE(saidYes);
}

