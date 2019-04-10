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

	std::this_thread::sleep_for(500ms);
	REQUIRE(b.IsThreadRunning());

	b.Stop();
	std::this_thread::sleep_for(500ms);
	REQUIRE(!b.IsThreadRunning());
}
