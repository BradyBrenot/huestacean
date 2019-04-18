#define CATCH_CONFIG_RUNNER

#include "catch/catch.hpp"

#include <QCoreApplication>

int main(int argc, char* argv[])
{
	QCoreApplication a(argc, argv);

	QCoreApplication::setOrganizationName("Brady Brenot");
	QCoreApplication::setOrganizationDomain("bradybrenot.com");
	QCoreApplication::setApplicationName("Huestacean Test");

	int result = Catch::Session().run(argc, argv);

	return (result < 0xff ? result : 0xff);
}
