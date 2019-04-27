#include "frontend/frontend.h"

#include "hue/hue.h"
#include "razer/razer.h"

#include <QSignalBlocker>

Frontend::Frontend(std::shared_ptr<Backend> inBackend) 
	: FrontendSimpleSource(nullptr)
	, m_Backend(inBackend)
	, changeIgnoreRequests(0)
{
	//Set state based on Backend

	//Listen for changes to Backend and update accordingly
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


	//Listen for changes coming from network to copy down to backend
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
}

void Frontend::BackendActiveSceneChanged()
{
	QSignalBlocker Bl(this);

	//@TODO: Update Active Scene
}
void Frontend::BackendScenesChanged()
{
	QSignalBlocker Bl(this);

	//@TODO: Update Scenes
}
void Frontend::BackendHueChanged()
{
	QSignalBlocker Bl(this);

	//@TODO: Update Bridges
	BackendDevicesChanged();
}

void Frontend::BackendRazerChanged()
{
	QSignalBlocker Bl(this);

	//@TODO: Update Razer
	BackendDevicesChanged();
}

void Frontend::BackendDevicesChanged()
{
	QSignalBlocker Bl(this);

	//@TODO: Update Devices
}

void Frontend::RemoteActiveSceneIndexChanged(qint32 ActiveSceneIndex)
{
	ScopedIgnoreChanges Ig(this);
	m_Backend->SetActiveScene(static_cast<int>(ActiveSceneIndex));
}
void Frontend::RemoteScenesChanged(QList<SceneInfo> Scenes)
{
	ScopedIgnoreChanges Ig(this);

	//@TODO: COPY
}
void Frontend::RemoteDevicesChanged(QList<DeviceInfo> Devices)
{
	ScopedIgnoreChanges Ig(this);

	//resend it? What is wrong with client.
	//@TODO: make this impossible
	BackendDevicesChanged();
}
void Frontend::RemoteBridgesChanged(QList<BridgeInfo> Bridges)
{
	ScopedIgnoreChanges Ig(this);

	//@TODO: COPY
}
void Frontend::RemoteRazerChanged(QList<RazerInfo> Razer)
{
	ScopedIgnoreChanges Ig(this);

	//@TODO: COPY
}