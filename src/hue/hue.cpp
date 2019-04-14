#include "hue/hue.h"

using namespace Hue;

Provider::Provider() :
	DeviceProvider(ProviderType::Hue)
{

}

void Provider::Update(const LightUpdateParams& Params)
{

}

std::vector<DevicePtr> Provider::GetDevices()
{
	return std::vector<DevicePtr>();
}