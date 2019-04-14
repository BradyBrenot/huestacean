#pragma once

#include "backend/deviceprovider.h"

namespace Hue
{
	class Bridge
	{

	};

	class Provider : public DeviceProvider
	{
	public:
		Provider();

		virtual void Update(const LightUpdateParams& Params) override;
		virtual std::vector<DevicePtr> GetDevices() override;
	};
};