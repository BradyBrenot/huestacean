#include "razer/razer.h"
#include "razer/razerdevices.h"
#include "razer/chroma.h"

using namespace Razer;

Provider::Provider()
	: DeviceProvider(ProviderType::Razer)
{
	devices.push_back(std::make_shared<GenericKeyboard>());
	devices.push_back(std::make_shared<GenericMouse>());
	devices.push_back(std::make_shared<GenericKeypad>());
	devices.push_back(std::make_shared<GenericHeadset>());
	devices.push_back(std::make_shared<GenericChromaLink>());
	devices.push_back(std::make_shared<GenericMousepad>());
}

Provider::~Provider()
{

}

void Provider::Update(const LightUpdateParams& Params)
{
	auto deviceIt = Params.devicesBegin;
	auto colorsIt = Params.colorsBegin;

	while (deviceIt != Params.devicesEnd)
	{
		auto* chromaDevice = dynamic_cast<ChromaDeviceBase*>((*deviceIt).get());
		chromaDevice->Upload(colorsIt, deviceIt, *Sdk);
	}
}

std::vector<DevicePtr> Provider::GetDevices()
{
	return devices;
}

DevicePtr Provider::GetDeviceFromUniqueId(std::string id) const
{
	//Razer|UniqueId
	auto deviceString = id.substr(ProviderType{ ProviderType::Hue }.ToString().size(), id.size());
	for (auto& d : devices)
	{
		if (d->GetUniqueId() == id)
		{
			return d;
		}
	}

	return nullptr;
}

void Provider::Start()
{
	Sdk = std::make_unique<Chroma>();
}
void Provider::Stop()
{
	Sdk = nullptr;
}