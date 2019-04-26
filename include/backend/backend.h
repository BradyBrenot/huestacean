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

#include "common/changelistenernotifier.h"

//Backend runs the flippin' show.

class Backend : public ChangeListenerNotifier
{
public:
	Backend();
	~Backend();

	//////////////////////////////////////////////////////////////////////////

	static const int EVENT_SCENES_CHANGED = 1;
	static const int EVENT_ACTIVE_SCENE_CHANGED = 2;

	//////////////////////////////////////////////////////////////////////////

	void Start();
	bool IsRunning();
	void Stop();

	const std::vector<Scene> GetScenes();
	void SetActiveScene(int sceneIndex);

	class BackendWriter
	{
	public:
		BackendWriter() = delete;
		BackendWriter(const BackendWriter& x) = delete;
		BackendWriter(BackendWriter&& x) = delete;

		explicit BackendWriter(Backend* inBackend) : b(inBackend), lock(inBackend->scenesMutex)
		{

		}

		~BackendWriter() { b->scenesAreDirty = true; b->NotifyListeners(Backend::EVENT_SCENES_CHANGED); }
		
		std::vector<Scene> GetScenes() const
		{
			return b->scenes;
		};

		std::vector<Scene>& GetScenesMutable()
		{
			return b->scenes;
		};

		void Load()
		{
			b->Load();
		}
		void Save()
		{
			b->Save();
		}

	private:
		std::scoped_lock<std::shared_mutex> lock;
		Backend* b;
	};

	BackendWriter GetWriter();

	std::unique_ptr<DeviceProvider>& GetDeviceProvider(ProviderType type);

private:

	//scenesMutex needs to be locked before calling. BackendWriter ensures this.
	void Load();
	void Save();

	std::atomic_bool stopRequested;
	std::thread thread;

	std::shared_mutex scenesMutex;
	std::vector<Scene> scenes;

	std::atomic_int activeSceneIndex;
	std::atomic_bool scenesAreDirty;

	std::unordered_map<ProviderType, std::unique_ptr<DeviceProvider> > deviceProviders;
};