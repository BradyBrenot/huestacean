#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"
#include "backend/backend.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

int main(int argc, char* argv[])
{
	int result = Catch::Session().run(argc, argv);

	return (result < 0xff ? result : 0xff);
}

TEST_CASE("can start and stop Backend", "") {
	Backend b;

	b.Start();
	REQUIRE(b.IsRunning());

	b.Stop();
	REQUIRE(!b.IsRunning());
}

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

	auto& hue = b.GetDeviceProvider(ProviderType::Hue);
	REQUIRE(hue.get() != nullptr);

	SECTION("Hue can find at least 1 device in 5 seconds") {
		std::this_thread::sleep_for(5s);
		REQUIRE(hue->GetDevices().size() > 0);
	}
}
