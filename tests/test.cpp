#define CATCH_CONFIG_RUNNER
#include "catch/catch.hpp"
#include "backend/backend.h"

int main(int argc, char* argv[])
{
	int result = Catch::Session().run(argc, argv);

	return (result < 0xff ? result : 0xff);
}

TEST_CASE("can construct a Backend", "") {

	Backend b;
}
