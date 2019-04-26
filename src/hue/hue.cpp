#include "hue/hue.h"
#include "common/math.h"

#include "hue/bridgediscovery.h"
#include <sstream>
#include <memory>

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
	if (Params.devicesDirty)
	{
		perBridgeUpdateInfo.clear();
		
		for (auto it = Params.devicesBegin; it != Params.devicesEnd; ++it)
		{
			auto& updateInfo = perBridgeUpdateInfo.emplace_back();
			auto* asLight = dynamic_cast<Light*>((*it).get());
			
			for (int i = 0; i < bridges.size(); ++i)
			{
				if (bridges[i]->id == asLight->bridgeid)
				{
					updateInfo.bridgeIndex = i;
					break;
				}
			}

			updateInfo.deviceIndex = asLight->id;
		}

		for (const auto& b : bridges)
		{
			std::vector<DevicePtr> BridgeLights;
			for (auto it = Params.devicesBegin; it != Params.devicesEnd; ++it)
			{
				auto l = dynamic_cast<Light*>((*it).get());
				if (l->bridgeid == b->id)
				{
					BridgeLights.push_back(*it);
				}
			}

			if (BridgeLights.size() > 0)
			{
				b->StartFromUpdateThread(BridgeLights);
			}
		}
	}

	int currentBridgeIndex = -1;
	int i = 0;
	auto colorsIt = Params.colorsBegin;
	while(colorsIt != Params.colorsEnd
		&& i < perBridgeUpdateInfo.size())
	{
		currentBridgeIndex = perBridgeUpdateInfo[i].bridgeIndex;
		std::vector<std::tuple<uint32_t, XyyColor> > lightsToUpload;

		while (colorsIt != Params.colorsEnd
			&& i < perBridgeUpdateInfo.size()
			&& perBridgeUpdateInfo[i].bridgeIndex == currentBridgeIndex)
		{
			lightsToUpload.push_back({ perBridgeUpdateInfo[i].deviceIndex, XyyColor{*colorsIt} });

			i++;
			colorsIt++;
		}

		if (lightsToUpload.size() > 0)
		{
			bridges[currentBridgeIndex]->Upload(lightsToUpload);
		}
	}
}

std::vector<DevicePtr> Provider::GetDevices()
{
	return std::vector<DevicePtr>();
}

bool Provider::compare(DeviceInScene a, DeviceInScene b)
{
	auto aL = dynamic_cast<Light*>(a.device.get());
	auto bL = dynamic_cast<Light*>(b.device.get());

	if (!aL) {
		return false;
	}
	else if (!bL) {
		return true;
	}

	//Find and compare bridge index, id
	
	auto findBridgeIndex = [&](std::string id) {
		int i = 0;

		for (auto& bridge : bridges)
		{
			if (bridge->id == id)
			{
				return i;
			}
		}
		return -1;
	};
	
	auto aBridgeIndex = findBridgeIndex(aL->bridgeid);
	auto bBridgeIndex = findBridgeIndex(bL->bridgeid);

	return aBridgeIndex < bBridgeIndex
		&& aL->id < bL->id;
}

DevicePtr Provider::GetDeviceFromUniqueId(std::string id)
{
	//Hue|Bridge|Device
	auto bridgeDeviceString = id.substr(ProviderType{ ProviderType::Hue }.ToString().size(), id.size());
	auto pipeIndex = bridgeDeviceString.find('|');
	auto bridgeString = bridgeDeviceString.substr(0, pipeIndex);
	//auto deviceString = bridgeDeviceString.substr(pipeIndex + 1, bridgeString.size());

	for (auto& b : bridges)
	{
		if (b->id == bridgeString)
		{
			for (auto& d : b->devices)
			{
				if (d->GetUniqueId() == id)
				{
					return d;
				}
			}
		}
	}

	auto dummyLight = std::make_shared<Light>();
	dummyLight->name = "ORPHANED LIGHT";
	return dummyLight;
}

void Provider::Start()
{
	
}
void Provider::Stop()
{
	for (const auto& b : bridges)
	{
		b->Stop();
	}
}
void Provider::UpdateThreadCleanup()
{
	for (const auto& b : bridges)
	{
		b->UpdateThreadCleanup();
	}
}

void Provider::SearchForBridges(std::vector<std::string> manualAddresses, bool doScan)
{
	discovery->Search(manualAddresses, doScan, [&](const std::vector<Bridge>& foundBridges) {
		for (const auto& found : foundBridges)
		{
			[&]()
			{
				//Look for an existing, matching bridge
				for (const auto& known : bridges)
				{
					if (known->id == found.id)
					{
						if (known->GetStatus() == Bridge::Status::Undiscovered)
						{
							if (known->clientkey.empty())
							{
								*known.get() = found;
							}
							else
							{
								known->SetStatus(Bridge::Status::Discovered);
							}
						}
						return;
					}
				}

				//If not found, create a new bridge
				bridges.push_back(std::make_shared<Bridge>(found));
				NotifyListeners(EVENT_BRIDGES_CHANGED);
			}();
		}

		for (const auto& bridge : bridges)
		{
			if (bridge->GetStatus() == Bridge::Status::Discovered)
			{
				bridge->Connect();
			}
		}
	});
}

const std::vector<std::shared_ptr<class Bridge>>& Provider::GetBridges()
{
	return bridges;
}

void Provider::Save(QSettings& settings)
{
	settings.beginGroup(ProviderType(ProviderType::Hue).ToString().c_str());

	settings.beginWriteArray("bridges");

	int i = 0;
	for (const auto& b : bridges)
	{
		if (b->id.empty())
		{
			continue;
		}

		settings.setArrayIndex(i++);

		settings.setValue("id", b->id.c_str());
		settings.setValue("address", b->address);
		settings.setValue("username", b->username.c_str());
		settings.setValue("clientkey", b->clientkey.c_str());
		settings.setValue("friendlyName", b->friendlyName.c_str());

		settings.beginWriteArray("devices");
		int j = 0;
		for (const auto& d : b->devices)
		{
			settings.setArrayIndex(j++);

			settings.setValue("uniqueid", d->uniqueid.c_str());
			settings.setValue("id", d->id);
			settings.setValue("bridgeid", d->bridgeid.c_str());
			settings.setValue("name", d->name.c_str());
			settings.setValue("type", d->type.c_str());
			settings.setValue("productname", d->productname.c_str());
		}
		settings.endArray();
	}

	settings.endArray();

	settings.endGroup();
}
void Provider::Load(QSettings& settings)
{
	if (bridges.size() > 0)
	{
		//don't load overtop of existing data
		return;
	}

	settings.beginGroup(ProviderType(ProviderType::Hue).ToString().c_str());

	//for each bridge
	// add a bridge, read bridge properties (id, username etc.)
	// for each device on the bridge
	//  add a device, read id, other properties. All the properties

	int bridgesSize = settings.beginReadArray("bridges");
	for (int i = 0; i < bridgesSize; ++i)
	{
		settings.setArrayIndex(i);

		auto& b = bridges.emplace_back();
		b = std::make_shared<Bridge>(qnam, 
			std::string(settings.value("id").toString().toUtf8()), 
			settings.value("address").toUInt());

		b->username = std::string(settings.value("username").toString().toUtf8());
		b->clientkey = std::string(settings.value("clientkey").toString().toUtf8());
		b->friendlyName = std::string(settings.value("friendlyName").toString().toUtf8());

		int devicesSize = settings.beginReadArray("devices");
		for (int j = 0; j < devicesSize; ++j)
		{
			settings.setArrayIndex(j);

			auto& l = b->devices.emplace_back();
			l = std::make_shared<Light>();

			l->uniqueid = settings.value("uniqueid").toString().toUtf8();
			l->id = settings.value("id").toUInt();

			l->bridgeid = settings.value("bridgeid").toString().toUtf8();
			l->name = settings.value("name").toString().toUtf8();
			l->type = settings.value("type").toString().toUtf8();
			l->productname = settings.value("productname").toString().toUtf8();
		}
		settings.endArray();
	}
	settings.endArray();

	settings.endGroup();

	NotifyListeners(EVENT_BRIDGES_CHANGED);
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

std::string Light::GetUniqueIdInternal() const
{
	std::stringstream ss;
	if (!uniqueid.empty())
	{
		ss << bridgeid << "|" << uniqueid;
		return ss.str();
	}

	ss << bridgeid << "|" << name << type;
	return ss.str();
}