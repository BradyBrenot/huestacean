#include "catch/catch.hpp"
#include "backend/backend.h"
#include "hue/hue.h"
#include "hue/bridge.h"
#include "effects/effects.h"

#include <iostream>

#include <memory>
#include <QTest>

TEST_CASE("Razer device provider is OK", "[.][razer]") {
	Backend b;

	auto* razer = b.GetDeviceProvider(ProviderType::Razer);

	SECTION("Razer exists") {
		REQUIRE(razer != nullptr);
	}

	SECTION("Razer's ProviderType is right") {
		REQUIRE(razer->GetType() == ProviderType::Razer);
	}

	SECTION("Razer has some devices") {
		REQUIRE(razer->GetDevices().size() > 0);
	}
}

TEST_CASE("Razer can animate", "[.][razer]") {
	Backend b;

	{
		auto sr = b.GetWriter();
		sr.Load();
	}

	{
		auto& razer = b.razer;
		REQUIRE(razer->GetDevices().size() > 0);

		auto sr = b.GetWriter();
		auto& scenes = sr.GetScenesMutable();

		Scene s;
		auto devices = razer->GetDevices();

		int i = 0;
		for (auto& d : devices)
		{
			DeviceInScene& dis = s.devices.emplace_back();
			dis.device = d;
			dis.transform.location.x = s.size.x * (i / devices.size());
			dis.transform.location.y = s.size.y * (i / devices.size());
			dis.transform.location.z = s.size.z * (i / devices.size());
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
		std::cout << "Are the razer lights animating? [Y/N]: ";
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

