#pragma once

#include "backend/deviceprovider.h"
#include "common/device.h"
#include "common/changelistenernotifier.h"

namespace Razer
{
	class Chroma;

	class Provider : public DeviceProvider, public ChangeListenerNotifier
	{
	public:
		Provider();
		virtual ~Provider();

		virtual void Update(const LightUpdateParams& Params) override; //called from worker thread!
		virtual std::vector<DevicePtr> GetDevices() override;
		virtual DevicePtr GetDeviceFromUniqueId(std::string id) const override;

		virtual void Start() override;
		virtual void Stop() override;

	private:
		std::vector<DevicePtr> devices;

		std::unique_ptr<Chroma> Sdk;
	};
};