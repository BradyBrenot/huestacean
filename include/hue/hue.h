#pragma once

#include "backend/deviceprovider.h"

namespace Hue
{
	struct Bridge
	{
		uint32_t address;
		std::string id;
		std::string username;
		std::string clientkey;
		std::string friendlyName;

		enum class Status : uint8_t
		{
			Undiscovered,
			Discovered,
			WantsLink,
			Connected
		};
		Status status;
	};

	class Provider : public DeviceProvider
	{
	public:
		Provider();

		virtual void Update(const LightUpdateParams& Params) override;
		virtual std::vector<DevicePtr> GetDevices() override;

	private:
		std::shared_ptr<class QNetworkAccessManager> qnam;
	};
};