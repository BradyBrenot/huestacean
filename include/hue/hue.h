#pragma once

#include "backend/deviceprovider.h"
#include "common/device.h"

class QNetworkAccessManager;

namespace Hue
{
	class Bridge;
	class BridgeDiscovery;

	class Provider : public DeviceProvider
	{
	public:
		Provider();

		virtual void Update(const LightUpdateParams& Params) override; //called from worker thread!
		virtual std::vector<DevicePtr> GetDevices() override;
		virtual DevicePtr GetDeviceFromUniqueId(std::string id) override;

		void SearchForBridges(std::vector<std::string> manualAddresses, bool doScan);
		const std::vector<std::shared_ptr<class Bridge>>& GetBridges();

		virtual void Save(QSettings& settings) override;
		virtual void Load(QSettings& settings) override;

	private:
		std::shared_ptr<QNetworkAccessManager> qnam;
		std::vector<std::shared_ptr<Bridge>> bridges;
		std::shared_ptr <BridgeDiscovery> discovery;
	};

	constexpr uint32_t INVALID_ID = 0xffffffff;

	class Light : public Device
	{
	public:
		// I don't know if this is guaranteed; API doesn't say it's optional, 
		// but then it shows an example return without one
		std::string uniqueid;
		uint32_t id;

		std::string bridgeid;
		std::string name;
		std::string type;
		std::string productname;
		bool reachable;

		Light();
#if 0  //why would I need a copy ctor...
		Light(const Light& l);
#endif

		virtual std::vector<Math::Box> GetLightBoundingBoxes() const override;

	protected:
		virtual std::string GetUniqueIdInternal() const override;
	};
};