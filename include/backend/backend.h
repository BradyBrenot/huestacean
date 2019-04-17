#pragma once

#include <atomic>
#include <thread>
#include <shared_mutex>
#include <memory>
#include <vector>
#include <unordered_map>

#include "common/providertype.h"
#include "deviceprovider.h"
#include "common/scene.h"

//Backend runs the flippin' show.

class Backend
{
public:
	Backend();
	~Backend();

	void Start();
	bool IsRunning();
	void Stop();

	const std::vector<Scene> GetScenes();
	void SetActiveScene(int sceneIndex);

	class ScenesWriter
	{
	public:
		ScenesWriter() = delete;
		ScenesWriter(const ScenesWriter& x) = delete;
		ScenesWriter(ScenesWriter&& x) = delete;

		explicit ScenesWriter(Backend* inBackend) : b(inBackend), lock(inBackend->scenesMutex)
		{

		}

		~ScenesWriter() { b->scenesAreDirty = true; }
		
		std::vector<Scene>& GetScenesMutable()
		{
			return b->scenes;
		};

	private:
		std::scoped_lock<std::shared_mutex> lock;
		Backend* b;
	};

	ScenesWriter GetScenesWriter();

	std::unique_ptr<DeviceProvider>& GetDeviceProvider(ProviderType type);

	void Load();
	void Save();

private:
	std::atomic_bool stopRequested;
	std::thread thread;

	std::shared_mutex scenesMutex;
	std::vector<Scene> scenes;

	std::atomic_int activeSceneIndex;
	std::atomic_bool scenesAreDirty;

	std::unordered_map<ProviderType, std::unique_ptr<DeviceProvider> > deviceProviders;
};