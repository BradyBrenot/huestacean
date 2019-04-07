#define CATCH_CONFIG_EXTERNAL_INTERFACES
#include "catch/catch.hpp"

#include "huestaceand.h"
#include "server.h"

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include "rpc.grpc.pb.h"

#include <QSignalSpy>
#include <QSharedPointer>
#include <QElapsedTimer>
#include <QThread>

static void doDeleteLater(QObject *obj)
{
	obj->deleteLater();
}

static QSharedPointer<Server> apiTestServer;
static int testPort = 55610;

struct ApiTestListener : Catch::TestEventListenerBase {

	using TestEventListenerBase::TestEventListenerBase; // inherit constructor

	virtual void testCaseStarting(Catch::TestCaseInfo const& testInfo) override {
		apiTestServer = QSharedPointer<Server>(new Server(nullptr, ++testPort), doDeleteLater);
		QSignalSpy startedSpy(apiTestServer.data(), SIGNAL(started()));
		apiTestServer->start();
		bool startedSuccessfully = startedSpy.wait();
	}

	virtual void testCaseEnded(Catch::TestCaseStats const& testCaseStats) override {
		QSignalSpy stoppedSpy(apiTestServer.data(), SIGNAL(finished()));
		apiTestServer->stop();
		stoppedSpy.wait();
		apiTestServer.clear();
	}
};
CATCH_REGISTER_LISTENER(ApiTestListener)

TEST_CASE("can use the API", "") {
	auto server = apiTestServer;
	REQUIRE(server->isRunning());

	//////////////////////////////////////////////////////////////////////////
	// Connect
	std::string str = (QString("localhost:") + QString::number(testPort)).toStdString();
	auto channel = grpc::CreateChannel(str.c_str(), grpc::InsecureChannelCredentials());
	auto stub = libhuestacean::HuestaceanServer::NewStub(channel);

	SECTION("can enumerate device archetypes") {
		grpc::ClientContext c;
		libhuestacean::GetDeviceArchetypesRequest req;
		libhuestacean::GetDeviceArchetypesResponse res;

		auto status = stub->GetDeviceArchetypes(&c, req, &res);
		CAPTURE(status.error_code());
		CAPTURE(status.error_message());
		CAPTURE(status.error_details());
		REQUIRE(status.ok());
		REQUIRE(res.device_archetypes_size() > 0);
	}

	SECTION("can enumerate device provider archetypes") {
		grpc::ClientContext c;
		libhuestacean::GetDeviceProviderArchetypesRequest req;
		libhuestacean::GetDeviceProviderArchetypesResponse res;

		auto status = stub->GetDeviceProviderArchetypes(&c, req, &res);
		CAPTURE(status.error_code());
		CAPTURE(status.error_message());
		CAPTURE(status.error_details());
		REQUIRE(status.ok());

		REQUIRE(res.provider_archetypes_size() > 0);
	}

	SECTION("can enumerate device providers") {
		grpc::ClientContext c;
		libhuestacean::GetDeviceProvidersRequest req;
		libhuestacean::GetDeviceProvidersResponse res;

		auto status = stub->GetDeviceProviders(&c, req, &res);
		CAPTURE(status.error_code());
		CAPTURE(status.error_message());
		CAPTURE(status.error_details());
		REQUIRE(status.ok());

		REQUIRE(res.providers_size() > 0);
	}

	SECTION("can enumerate devices") {
		grpc::ClientContext c;
		libhuestacean::GetDevicesRequest req;
		libhuestacean::GetDevicesResponse res;

		auto status = stub->GetDevices(&c, req, &res);
		CAPTURE(status.error_code());
		CAPTURE(status.error_message());
		CAPTURE(status.error_details());
		REQUIRE(status.ok());

		REQUIRE(res.devices_size() > 0);
	}

	SECTION("can enumerate rooms") {
		grpc::ClientContext c;
		libhuestacean::GetRoomsRequest req;
		libhuestacean::GetRoomsResponse res;

		auto status = stub->GetRooms(&c, req, &res);
		CAPTURE(status.error_code());
		CAPTURE(status.error_message());
		CAPTURE(status.error_details());
		REQUIRE(status.ok());

		REQUIRE(res.rooms_size() > 0);
	}
}