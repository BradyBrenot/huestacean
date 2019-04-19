#include "catch/catch.hpp"
#include "backend/backend.h"
#include "hue/hue.h"
#include "hue/bridge.h"

#include <memory>
#include <QTest>

TEST_CASE("a Backend has a Hue device provider", "[hue]") {
	Backend b;

	auto& hue = b.GetDeviceProvider(ProviderType::Hue);
	REQUIRE(hue.get() != nullptr);

	SECTION("Hue's ProviderType is right") {
		REQUIRE(hue->GetType() == ProviderType::Hue);
	}
}

TEST_CASE("the Hue device provider can connect with bridges", "[hue]") {

	SECTION("Finds a bridge from scratch, links it, and finds devices on it") {
		Backend b;

		auto& dp = b.GetDeviceProvider(ProviderType::Hue);

		REQUIRE(dp.get() != nullptr);

		Hue::Provider * hue = dynamic_cast<Hue::Provider*>(dp.get());
		REQUIRE(hue != nullptr);

		hue->SearchForBridges(std::vector<std::string>(), true);

		REQUIRE(QTest::qWaitFor([&]() { return hue->GetBridges().size() > 0; }, 5000));

		auto& bridges = hue->GetBridges();
		bridges[0]->Connect();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->GetStatus() == Hue::Bridge::Status::Connected; }, 10000));

		bridges[0]->RefreshDevices();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->devices.size() > 0; }, 2000));

		auto sr = b.GetWriter();
		sr.Save();
	}

	SECTION("Connects to a bridge from a previous test using loaded settings") {
		Backend b;
		auto sr = b.GetWriter();
		sr.Load();

		auto& dp = b.GetDeviceProvider(ProviderType::Hue);
		Hue::Provider* hue = dynamic_cast<Hue::Provider*>(dp.get());

		hue->SearchForBridges(std::vector<std::string>(), true);

		REQUIRE(QTest::qWaitFor([&]() { return hue->GetBridges().size() > 0; }, 5000));

		auto& bridges = hue->GetBridges();
		bridges[0]->Connect();

		REQUIRE(QTest::qWaitFor([&]() { return bridges[0]->GetStatus() == Hue::Bridge::Status::Connected; }, 10000));
	}
}
