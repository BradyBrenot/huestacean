#include "catch/catch.hpp"
#include "backend/backend.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

TEST_CASE("can start and stop Backend", "") {
	Backend b;

	b.Start();
	REQUIRE(b.IsRunning());

	b.Stop();
	REQUIRE(!b.IsRunning());
}