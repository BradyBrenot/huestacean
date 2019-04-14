#pragma once

#include "backend/deviceprovider.h"

namespace Hue
{
	struct Bridge
	{
		std::string id;
		uint32_t address;
		
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

		Bridge(std::string inId, uint32_t inAddress) :
			id(inId), address(inAddress)
		{

		}
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