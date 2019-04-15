#include "hue/hue.h"
#include "common/math.h"

using namespace Hue;
using namespace Math;

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