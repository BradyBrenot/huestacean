#pragma once

#include "backend/deviceprovider.h"
#include "common/device.h"

#include "common/changelistenernotifier.h"

class QNetworkAccessManager;

namespace Hue
{
	class Bridge;
	class BridgeDiscovery;

	struct BridgeUpdateInfo
	{
		int bridgeIndex;
		uint32_t deviceIndex;
	};

	class Provider : public DeviceProvider, public ChangeListenerNotifier
	{
	public:
		Provider();
		virtual ~Provider();

		//////////////////////////////////////////////////////////////////////////

		static const int EVENT_BRIDGES_CHANGED = 1;
		static const int EVENT_A_BRIDGE_CHANGED = 2;

		//////////////////////////////////////////////////////////////////////////

		virtual void Update(const LightUpdateParams& Params) override; //called from worker thread!
		virtual std::vector<DevicePtr> GetDevices() const override;
		virtual bool compare(DeviceInScene a, DeviceInScene b) override;
		virtual DevicePtr GetDeviceFromUniqueId(std::string id) const override;

		virtual void Start() override;
		virtual void Stop() override;
		virtual void UpdateThreadCleanup() override;

		void SearchForBridges(std::vector<std::string> manualAddresses, bool doScan);
		const std::vector<std::shared_ptr<class Bridge>>& GetBridges();

		virtual void Save(QSettings& settings) override;
		virtual void Load(QSettings& settings) override;

	private:
		std::shared_ptr<QNetworkAccessManager> qnam;
		std::vector<std::shared_ptr<Bridge>> bridges;
		std::vector<int> bridgeListenerIds;
		std::shared_ptr <BridgeDiscovery> discovery;

		//vector contains vectors telling us which devices belong to which bridge, and have which index 
		std::vector<BridgeUpdateInfo> perBridgeUpdateInfo;
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