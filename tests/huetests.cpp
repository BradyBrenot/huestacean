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
	Backend b;

	auto& dp = b.GetDeviceProvider(ProviderType::Hue);

	REQUIRE(dp.get() != nullptr);

	Hue::Provider* hue = dynamic_cast<Hue::Provider*>(dp.get());
	REQUIRE(hue != nullptr);

	hue->SearchForBridges(std::vector<std::string>(), true);
	
	SECTION("Hue can find at least 1 bridge in 5 seconds") {
		QTest::qWait(5000);
		REQUIRE(hue->GetBridges().size() > 0);
	}

	SECTION("Finds a bridge, links it, and finds devices on it") {
		QTest::qWait(1000);

		auto& bridges = hue->GetBridges();
		REQUIRE(bridges.size() > 0);
		bridges[0]->Connect();
		QTest::qWait(10000);

		REQUIRE(bridges[0]->GetStatus() == Hue::Bridge::Status::Connected);

		bridges[0]->RefreshDevices();

		QTest::qWait(2000);

		REQUIRE(bridges[0]->Devices.size() > 0);
	}
}
