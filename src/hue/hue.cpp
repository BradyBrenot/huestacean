#include "hue/hue.h"
#include "common/math.h"

#include "hue/bridgediscovery.h"

using namespace Hue;
using namespace Math;

Provider::Provider() :
	DeviceProvider(ProviderType::Hue)
{
	qnam = std::make_shared<QNetworkAccessManager>();
	discovery = std::make_shared<BridgeDiscovery>(qnam);
}

void Provider::Update(const LightUpdateParams& Params)
{

}

std::vector<DevicePtr> Provider::GetDevices()
{
	return std::vector<DevicePtr>();
}
void Provider::SearchForBridges(std::vector<std::string> manualAddresses, bool doScan)
{
	discovery->Search(manualAddresses, doScan, [&](const std::vector<Bridge>& foundBridges) {
		for (const auto& found : foundBridges)
		{
			//Look for an existing, matching bridge
			for (const auto& known : bridges)
			{
				if (known->id == found.id)
				{
					if (known->GetStatus() == Bridge::Status::Undiscovered)
					{
						*known.get() = found;
					}
					return;
				}
			}

			//If not found, create a new bridge
			bridges.push_back(std::make_shared<Bridge>(found));
		}
	});
}

const std::vector<std::shared_ptr<class Bridge>>& Provider::GetBridges()
{
	return bridges;
}


Light::Light()
	: Device(ProviderType::Hue),
	uniqueid(""),
	id(INVALID_ID),
	name(""),
	type(""),
	productname(""),
	reachable(false)
{

}

#if 0 //why would I need a copy ctor...
Light::Light(const Light& l)
	: Device(ProviderType::Hue),
	uniqueid(l.uniqueid),
	id(l.id),
	name(l.name),
	type(l.type),
	productname(l.productname),
	reachable(l.reachable)
{

}
#endif

std::vector<Math::Box> Light::GetLightBoundingBoxes() const
{
	return { Math::Box{ {0, 0, 0}, { 1_m, 1_m, 1_m } } };
}