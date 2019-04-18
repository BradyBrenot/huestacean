#include "backend/backend.h"
#include "common/lightupdate.h"
#include "common/math.h"

#include "hue/hue.h"

#include <QSettings>

#include <chrono>
#include <thread>
#include <algorithm>

using namespace std::chrono_literals;
using namespace Math;

Backend::Backend() :
	stopRequested(false),
	thread(),
	scenesMutex(),
	scenes(),
	activeSceneIndex(0),
	scenesAreDirty(false),
	deviceProviders()
{
	deviceProviders.emplace(ProviderType::Hue, std::make_unique<Hue::Provider>());
}

Backend::~Backend()
{
	Stop();
}

void Backend::Start()
{
	//DOES NOT mutate scenes
	//DOES NOT read from scenes without locking scenesMutex

	if (IsRunning()) {
		return;
	}

	stopRequested = false;
	thread = std::thread([this] {
		Scene renderScene = scenes.size() > activeSceneIndex ? scenes[activeSceneIndex] : Scene();

		std::unordered_map<ProviderType, LightUpdateParams> lightUpdates;
		std::vector<HsluvColor> colors;
		std::vector<Box> boundingBoxes;
		std::vector<DevicePtr> devices;

		auto tick = [&](float deltaTime)
		{
			//Copy the new scene if necessary
			// @TODO multiple active scenes
			if (scenesAreDirty)
			{
				{
					std::scoped_lock lock(scenesMutex);
					int updateThreadSceneIndex = activeSceneIndex;
					renderScene = scenes.size() > updateThreadSceneIndex ? scenes[updateThreadSceneIndex] : Scene();
				}

				//Sort devices by ProviderType     
				std::sort(renderScene.devices.begin(), renderScene.devices.end(),
					[&](const DeviceInScene & a, const DeviceInScene & b) {
						if (a.device->GetType() == b.device->GetType())
						{
							return deviceProviders[a.device->GetType()]->compare(a, b);
						}
						else
						{
							return compare(a.device, b.device);
						}
					});

				//Query every device to fetch positions off it 
				//	+ Fill in big dumb non-sparse Devices array
				boundingBoxes.clear();
				devices.clear();

				for (const auto& d : renderScene.devices)
				{
					auto boxesToAppend = d.GetLightBoundingBoxes();
					auto devicesToAppend = std::vector<DevicePtr>(boxesToAppend.size(), d.device);

					boundingBoxes.insert(boundingBoxes.end(), boxesToAppend.begin(), boxesToAppend.end());
					devices.insert(devices.end(), devicesToAppend.begin(), devicesToAppend.end());
				}


				//Resize Colors
				colors.resize( boundingBoxes.size() );

				for (const auto& dp : deviceProviders)
				{
					auto update = lightUpdates[dp.first];
					update.boundingBoxesDirty = true;
					update.colorsDirty = true;
					update.devicesDirty = true;

					update.colorsBegin = colors.begin();
					update.devicesBegin = devices.begin();
					update.boundingBoxesBegin = boundingBoxes.begin();

					while (update.devicesBegin != devices.end() && (*update.devicesBegin)->GetType() != dp.first)
					{
						update.colorsBegin++;
						update.devicesBegin++;
						update.boundingBoxesBegin++;
					}

					update.colorsEnd = update.colorsBegin;
					update.devicesEnd = update.devicesBegin;
					update.boundingBoxesEnd = update.boundingBoxesBegin;

					while (update.devicesEnd != devices.end() && (*update.devicesEnd)->GetType() == dp.first)
					{
						update.colorsEnd++;
						update.devicesEnd++;
						update.boundingBoxesEnd++;
					}
				}
			}

			//Run effects
			for (auto& effect : renderScene.effects)
			{
				effect->Tick(deltaTime);
				effect->Update(boundingBoxes, colors);
			}

			//Send light data to device providers
			for (const auto& dp : deviceProviders)
			{
				dp.second->Update(lightUpdates[dp.first]);
			}

			//DONE
		};

		auto lastStart = std::chrono::high_resolution_clock::now();

		while (!stopRequested) {
			constexpr auto tickRate = 16.67ms;

			auto start = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>{ start - lastStart }.count();
			lastStart = start;

			tick(deltaTime);

			auto end = std::chrono::high_resolution_clock::now();

			//sleep to keep our tick rate right, or at least 1ms
			auto timeLeft = tickRate - (end - start);
			auto sleepForTime = timeLeft > 1ms ? timeLeft : 1ms;
			std::this_thread::sleep_for(sleepForTime);
		}
	});
}

bool Backend::IsRunning()
{
	return thread.joinable();
}

void Backend::Stop()
{
	if (!IsRunning()) {
		return;
	}

	stopRequested = true;
	thread.join();
}

const std::vector<Scene> Backend::GetScenes()
{
	std::scoped_lock lock(scenesMutex);
	return scenes;
}

Backend::ScenesWriter Backend::GetScenesWriter()
{
	return ScenesWriter(this);
}

std::unique_ptr<DeviceProvider>& Backend::GetDeviceProvider(ProviderType type)
{
	return deviceProviders[type];
}

void Backend::Save()
{
	QSettings settings;
	settings.clear();

	//let every DisplayProvider save first
	for (const auto& dp : deviceProviders)
	{
		dp.second->Save(settings);
	}

	//save scenes
	std::vector<Scene> scenesToSave = GetScenes();

	settings.beginWriteArray("scenes");
	int i = 0;
	for (const auto& scene : scenesToSave)
	{
		settings.setArrayIndex(i++);

		settings.beginWriteArray("effects");
		int j = 0;
		for (const auto& effect : scene.effects)
		{
			settings.setArrayIndex(j++);
			effect->Save(settings);

		}
		settings.endArray();

		settings.beginWriteArray("devices");
		j = 0;
		for (const auto& device : scene.devices)
		{
			settings.setArrayIndex(j++);
			settings.setProperty("id", device.device->GetUniqueId().c_str());

			settings.setProperty("t.x", device.transform.location.x);
			settings.setProperty("t.y", device.transform.location.y);
			settings.setProperty("t.z", device.transform.location.z);
			settings.setProperty("t.sx", device.transform.scale.x);
			settings.setProperty("t.sy", device.transform.scale.y);
			settings.setProperty("t.sz", device.transform.scale.z);
			settings.setProperty("t.p", device.transform.rotation.pitch);
			settings.setProperty("t.y", device.transform.rotation.yaw);
			settings.setProperty("t.r", device.transform.rotation.roll);
		}
		settings.endArray();
	}
	settings.endArray();
}

void Backend::Load()
{
	QSettings settings;

	//let every DisplayProvider load first
	for (const auto& dp : deviceProviders)
	{
		dp.second->Load(settings);
	}

	//load scenes
	auto sw = GetScenesWriter();

	settings.beginReadArray("scenes");
	int scenesSize = settings.beginReadArray("scenes");
	for (int i = 0; i < scenesSize; ++i)
	{
		settings.setArrayIndex(i);
		Scene& scene = scenes.emplace_back();

		int effectsSize = settings.beginReadArray("effects");
		for (int j = 0; j < effectsSize; ++j)
		{
			settings.setArrayIndex(j++);
			scene.effects.push_back(Effect::StaticLoad(settings));
		}
		settings.endArray();

		int devicesSize = settings.beginReadArray("devices");
		for (int j = 0; j < devicesSize; ++j)
		{
			settings.setArrayIndex(j++);
			
			std::string id = std::string(settings.value("id").toString().toUtf8());
			auto providerType = Device::GetProviderTypeFromUniqueId(id);

			std::shared_ptr<Device> d = deviceProviders[providerType]->GetDeviceFromUniqueId(id);
			if (d == nullptr)
			{
				continue;
			}

			DeviceInScene& dis = scene.devices.emplace_back();
			dis.device = d;
			
			dis.transform.location.x = settings.value("t.x").toDouble();
			dis.transform.location.x = settings.value("t.y").toDouble();
			dis.transform.location.x = settings.value("t.z").toDouble();
			dis.transform.scale.x = settings.value("t.sx").toDouble();
			dis.transform.scale.y = settings.value("t.sy").toDouble();
			dis.transform.scale.z = settings.value("t.sz").toDouble();
			dis.transform.rotation.pitch = settings.value("t.p").toDouble();
			dis.transform.rotation.yaw = settings.value("t.y").toDouble();
			dis.transform.rotation.roll = settings.value("t.r").toDouble();
		}
		settings.endArray();
	}
	settings.endArray();
}