#include "frontend/frontend.h"

#include "hue/hue.h"
#include "razer/razer.h"

#include <QSignalBlocker>

Frontend::Frontend(std::shared_ptr<Backend> inBackend) 
	: FrontendSimpleSource(nullptr)
	, m_Backend(inBackend)
	, changeIgnoreRequests(0)
{
	// Listen for changes to Backend and update accordingly
	scenesListenerId = m_Backend->RegisterListener([&]() {
		if (isIgnoringChanges()) {
			return;
		}

		BackendScenesChanged();
		}, { Backend::EVENT_SCENES_CHANGED });

	activeSceneListenerId = m_Backend->RegisterListener([&]() {
		if (isIgnoringChanges()) {
			return;
		}

		BackendActiveSceneChanged();
		}, { Backend::EVENT_ACTIVE_SCENE_CHANGED });

	Hue::Provider& hue = m_Backend->hue;
	hueListenerId = hue.RegisterListener([&]() {
		if (isIgnoringChanges()) {
			return;
		}

		BackendHueChanged();
	});

	Razer::Provider& razer = m_Backend->razer;
	razerListenerId = razer.RegisterListener([&]() {
		if (isIgnoringChanges()) {
			return;
		}

		BackendRazerChanged();
	});


	// Listen for changes coming from network to copy down to backend
	connect(this, SIGNAL(ActiveSceneIndexChanged(qint32)),
		this, SLOT(RemoteActiveSceneIndexChanged(qint32)));

	connect(this, SIGNAL(ScenesChanged(QList<SceneInfo>)),
		this, SLOT(RemoteScenesChanged(QList<SceneInfo>)));

	connect(this, SIGNAL(DevicesChanged(QList<DeviceInfo>)),
		this, SLOT(RemoteDevicesChanged(QList<DeviceInfo>)));

	connect(this, SIGNAL(BridgesChanged(QList<BridgeInfo>)),
		this, SLOT(RemoteBridgesChanged(QList<BridgeInfo>)));

	connect(this, SIGNAL(RazerChanged(QList<RazerInfo>)),
		this, SLOT(RemoteRazerChanged(QList<RazerInfo>)));

	// Set initial state
	BackendActiveSceneChanged();
	BackendScenesChanged();

	BackendHueChanged();
	BackendRazerChanged();

	BackendDevicesChanged();
}

void Frontend::StartUpdateLoop()
{
	m_Backend->Start();
}
void Frontend::StopUpdateLoop()
{
	m_Backend->Stop();
}

void Frontend::BackendActiveSceneChanged()
{
	QSignalBlocker Bl(this);
	setActiveSceneIndex(m_Backend->GetActiveScene());
}
void Frontend::BackendScenesChanged()
{
	QSignalBlocker Bl(this);
	
	auto w = m_Backend->GetWriter();

	QList<SceneInfo> newScenes;
	for (const auto& s : w.GetScenes())
	{
		newScenes.push_back(Scene_BackendToFrontend(s));
	}

	setScenes(newScenes);
}
void Frontend::BackendHueChanged()
{
	QSignalBlocker Bl(this);

	QList<BridgeInfo> newBridges;

	for (const auto& b : m_Backend->hue.GetBridges())
	{
		newBridges.push_back(Bridge_BackendToFrontend(b));
	}

	setBridges(newBridges);

	// Devices list probably changed
	BackendDevicesChanged();
}

void Frontend::BackendRazerChanged()
{
	QSignalBlocker Bl(this);

	RazerInfo newRazer;

	auto& razer = m_Backend->razer;
	auto& devices = razer.GetDevices();

	for (auto& d : devices)
	{
		newRazer.devices.push_back(Device_BackendToFrontend(d));
	}

	setRazer(newRazer);

	// Devices list probably changed
	BackendDevicesChanged();
}

void Frontend::BackendDevicesChanged()
{
	QSignalBlocker Bl(this);

	QList<DeviceInfo> newDevices;

	std::vector<DevicePtr> backendDevices;
	for (auto& dp : m_Backend->GetDeviceProviders())
	{
		auto dpDevices = dp.get().GetDevices();
		backendDevices.insert(backendDevices.end(), dpDevices.begin(), dpDevices.end());
	}

	setDevices(newDevices);
}

void Frontend::RemoteActiveSceneIndexChanged(qint32 ActiveSceneIndex)
{
	ScopedIgnoreChanges Ig(this);
	m_Backend->SetActiveScene(static_cast<int>(ActiveSceneIndex));
}
void Frontend::RemoteScenesChanged(QList<SceneInfo> Scenes)
{
	ScopedIgnoreChanges Ig(this);
	auto w = m_Backend->GetWriter();
	w.GetScenesMutable().clear();

	for (auto& s : Scenes)
	{
		w.GetScenesMutable().push_back(Scene_FrontendToBackend(s, *m_Backend));
	}
}
void Frontend::RemoteDevicesChanged(QList<DeviceInfo> Devices)
{
	//@TODO: make this impossible?
	ScopedIgnoreChanges Ig(this);
	BackendDevicesChanged();
}
void Frontend::RemoteBridgesChanged(QList<BridgeInfo> Bridges)
{
	//@TODO: make this impossible?
	ScopedIgnoreChanges Ig(this);
	BackendHueChanged();
}
void Frontend::RemoteRazerChanged(QList<RazerInfo> Razer)
{
	//@TODO: make this impossible?
	ScopedIgnoreChanges Ig(this);
	BackendRazerChanged();
}