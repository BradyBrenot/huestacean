#include "backend/backend.h"
#include "common/lightupdate.h"
#include "common/math.h"

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
	activeSceneIndex(-1),
	scenesAreDirty(false),
	hue(),
	razer()
{
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

	{
		for (const auto& dp : GetDeviceProviders())
		{
			dp.get().Start();
		}
	}
	

	stopRequested = false;
	thread = std::thread([this] {
		Scene renderScene;
		scenesAreDirty = true;

		std::unordered_map<ProviderType, LightUpdateParams> lightUpdates;
		std::vector<HsluvColor> colors;
		std::vector<Box> boundingBoxes;
		std::vector<DevicePtr> devices;

		auto tick = [&](std::chrono::duration<float> deltaTime)
		{
			//Copy the new scene if necessary
			// @TODO multiple active scenes
			if (scenesAreDirty)
			{
				{
					std::scoped_lock lock(scenesMutex);
					int updateThreadSceneIndex = activeSceneIndex;
					renderScene = scenes.size() > updateThreadSceneIndex ? scenes[updateThreadSceneIndex] : Scene();
					scenesAreDirty = false;
				}

				//Sort devices by ProviderType     
				std::sort(renderScene.devices.begin(), renderScene.devices.end(),
					[&](const DeviceInScene & a, const DeviceInScene & b) {
						if (a.device->GetType() == b.device->GetType())
						{
							auto* dp = GetDeviceProvider(a.device->GetType());
							if (!dp) {
								return false;
							}
							return dp->compare(a, b);
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

				for (const auto& dp : GetDeviceProviders())
				{
					auto& update = lightUpdates[dp.get().GetType()];
					update.boundingBoxesDirty = true;
					update.colorsDirty = true;
					update.devicesDirty = true;

					update.colorsBegin = colors.begin();
					update.devicesBegin = devices.begin();
					update.boundingBoxesBegin = boundingBoxes.begin();

					while (update.devicesBegin != devices.end() && (*update.devicesBegin)->GetType() != dp.get().GetType())
					{
						update.colorsBegin++;
						update.devicesBegin++;
						update.boundingBoxesBegin++;
					}

					update.colorsEnd = update.colorsBegin;
					update.devicesEnd = update.devicesBegin;
					update.boundingBoxesEnd = update.boundingBoxesBegin;

					while (update.devicesEnd != devices.end() && (*update.devicesEnd)->GetType() == dp.get().GetType())
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
			for (const auto& dp : GetDeviceProviders())
			{
				dp.get().Update(lightUpdates[dp.get().GetType()]);
			}

			//DONE
		};

		auto lastStart = std::chrono::high_resolution_clock::now();

		while (!stopRequested) {
			constexpr auto tickRate = 30ms;

			auto start = std::chrono::high_resolution_clock::now();
			auto deltaTime = std::chrono::duration<float>{ start - lastStart };
			lastStart = start;

			tick(deltaTime);

			auto end = std::chrono::high_resolution_clock::now();

			//sleep to keep our tick rate right, or at least 1ms
			auto timeLeft = tickRate - (end - start);
			auto sleepForTime = timeLeft > 1ms ? timeLeft : 1ms;
			std::this_thread::sleep_for(sleepForTime);
		}

		//Allow for any cleanup that MUST happen on this thread
		for (const auto& dp : GetDeviceProviders())
		{
			dp.get().UpdateThreadCleanup();
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

	for (const auto& dp : GetDeviceProviders())
	{
		dp.get().Stop();
	}
}

const std::vector<Scene> Backend::GetScenes()
{
	std::scoped_lock lock(scenesMutex);
	return scenes;
}

void Backend::SetActiveScene(int sceneIndex)
{
	activeSceneIndex = sceneIndex;
	NotifyListeners(EVENT_ACTIVE_SCENE_CHANGED);

	if (activeSceneIndex >= 0 && !IsRunning()) {
		Start();
	}
	else if (activeSceneIndex < 0 && IsRunning()) {
		Stop();
	}
}

Backend::BackendWriter Backend::GetWriter()
{
	return BackendWriter(this);
}

const DeviceProvider* Backend::GetDeviceProvider(ProviderType type) const
{
	switch (type.type)
	{
	case ProviderType::Hue:
		return &hue;
		break;
	case ProviderType::Razer:
		return razer;
		break;
	default:
		break;
	}
	return nullptr;
}

DeviceProvider* Backend::GetDeviceProvider(ProviderType type)
{
	return const_cast<DeviceProvider*>(const_cast<const Backend*>(this)->GetDeviceProvider(type));
}

std::vector<std::reference_wrapper<DeviceProvider>> Backend::GetDeviceProviders()
{
	#ifdef _WIN32
	return {hue, razer};
	#endif
}

DevicePtr Backend::GetDeviceFromUniqueId(std::string id) const
{
	auto providerType = Device::GetProviderTypeFromUniqueId(id);

	auto* dp = GetDeviceProvider(providerType);
	if (dp == nullptr)
	{
		return nullptr;
	}

	return dp->GetDeviceFromUniqueId(id);
}

void Backend::Save()
{
	QSettings settings;
	settings.clear();

	//let every DisplayProvider save first
	for (const auto& dp : GetDeviceProviders())
	{
		dp.get().Save(settings);
	}

	//save scenes
	std::vector<Scene> scenesToSave = scenes;

	settings.beginWriteArray("scenes");
	int i = 0;
	for (const auto& scene : scenesToSave)
	{
		settings.setArrayIndex(i++);

		settings.setValue("name", scene.name.c_str());

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
			settings.setValue("id", device.device->GetUniqueId().c_str());

			settings.setValue("t.x", device.transform.location.x);
			settings.setValue("t.y", device.transform.location.y);
			settings.setValue("t.z", device.transform.location.z);
			settings.setValue("t.sx", device.transform.scale.x);
			settings.setValue("t.sy", device.transform.scale.y);
			settings.setValue("t.sz", device.transform.scale.z);
			settings.setValue("t.p", device.transform.rotation.pitch);
			settings.setValue("t.y", device.transform.rotation.yaw);
			settings.setValue("t.r", device.transform.rotation.roll);
		}
		settings.endArray();
	}
	settings.endArray();
}

void Backend::Load()
{
	QSettings settings;

	//let every DisplayProvider load first
	for (const auto& dp : GetDeviceProviders())
	{
		dp.get().Load(settings);
	}

	//load scenes
	int scenesSize = settings.beginReadArray("scenes");
	for (int i = 0; i < scenesSize; ++i)
	{
		settings.setArrayIndex(i);
		Scene& scene = scenes.emplace_back();

		scene.name = std::string(settings.value("name").toString().toUtf8());

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
			auto d = GetDeviceFromUniqueId(id);

			DeviceInScene& dis = scene.devices.emplace_back();
			dis.device = d;
			
			dis.transform.location.x = settings.value("t.x").toDouble();
			dis.transform.location.y = settings.value("t.y").toDouble();
			dis.transform.location.z = settings.value("t.z").toDouble();
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