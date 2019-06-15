#pragma once

#include "rep_frontend.h"
#include "backend/backend.h"
#include "frontend/utility.h"
#include "frontend/frontendtypes.h"

#include <memory>

class Frontend : public FrontendSimpleSource
{
	Q_OBJECT

public:
	explicit Frontend(std::shared_ptr<Backend> inBackend);

public:
	virtual ~Frontend() {}

private:

	////////////////////////////////////////////////
	// Things changed on the backend (locally), so
	// update myself to let the replicas know

	void BackendActiveSceneChanged();
	void BackendScenesChanged();

	void BackendHueChanged();
	void BackendRazerChanged();

	void BackendDevicesChanged();

	////////////////////////////////////////////////

	std::shared_ptr<Backend> m_Backend;

	int scenesListenerId;
	int activeSceneListenerId;
	int hueListenerId;
	int razerListenerId;

	class ScopedIgnoreChanges
	{
		Frontend* m_Fe;

	public:
		ScopedIgnoreChanges() = delete;

		ScopedIgnoreChanges(Frontend* Fe) : m_Fe(Fe)
		{
			m_Fe->changeIgnoreRequests++;
		}

		~ScopedIgnoreChanges()
		{
			m_Fe->changeIgnoreRequests--;
		}
	};


	int changeIgnoreRequests;
	bool isIgnoringChanges() {
		return changeIgnoreRequests > 0;
	}

private slots:

	///////////////////////////////////////////////////////
	// Things changed on a replica (remote or local), so 
	// update the backend to match the new expected state

	void RemoteActiveSceneIndexChanged(qint32 ActiveSceneIndex);
	void RemoteScenesChanged(QList<SceneInfo> Scenes);
	void RemoteDevicesChanged(QList<DeviceInfo> Devices);
	void RemoteBridgesChanged(QList<BridgeInfo> Bridges);
	void RemoteRazerChanged(QList<RazerInfo> Razer);

	///////////////////////////////////////////////////////
};

class FrontendQmlReplica : public FrontendReplica
{
	Q_OBJECT

	Q_PROPERTY(QList<QObject*> ScenesList READ ScenesList NOTIFY ScenesListChanged)
	Q_PROPERTY(QVariantList DevicesList READ DevicesList NOTIFY DevicesListChanged)
	Q_PROPERTY(QVariantList BridgesList READ BridgesList NOTIFY BridgesListChanged)

	template<typename T>
	static void doDeleteLater(T* obj)
	{
		obj->deleteLater();
	}

public:

	QList<QObject*> ScenesList() const
	{
		return makeQObjectList(m_ScenesList);
	}
	QVariantList DevicesList() const
	{
		return makeVariantList(Devices());
	}
	QVariantList BridgesList() const
	{
		return makeVariantList(Bridges());
	}

	FrontendQmlReplica() : FrontendReplica()
	{
		connect(this, SIGNAL(ScenesChanged(QList<SceneInfo>)),
			this, SLOT(OnScenesChanged(QList<SceneInfo>)));
		connect(this, SIGNAL(DevicesChanged(QList<DeviceInfo>)),
			this, SLOT(OnDevicesChanged(QList<DeviceInfo>)));
		connect(this, SIGNAL(BridgesChanged(QList<BridgeInfo>)),
			this, SLOT(OnBridgesChanged(QList<BridgeInfo>)));

		connect(this, SIGNAL(stateChanged(State, State)),
			this, SLOT(OnStateChanged()));
	}

signals:
	void ScenesListChanged();
	void DevicesListChanged();
	void BridgesListChanged();

public slots:
	void OnStateChanged()
	{
		//I don't know, it looks like QtRO doesn't fire the "On____Changed" events on
		//connection established, which means QML doesn't know that things actually
		//changed.

		emit ScenesChanged(Scenes());
		emit DevicesChanged(Devices());
		emit BridgesChanged(Bridges());
		emit RazerChanged(Razer());
		emit ActiveSceneIndexChanged(ActiveSceneIndex());
		emit IsRunningChanged(IsRunning());
	}
	void OnScenesChanged(QList<SceneInfo> InScenes)
	{
		m_ScenesList.clear();

		for (auto& s : InScenes)
		{
			m_ScenesList.push_back(QSharedPointer<SceneInfo>(new SceneInfo(s), doDeleteLater<SceneInfo>));
		}

		emit ScenesListChanged();
	}
	void OnDevicesChanged(QList<DeviceInfo> Devices)
	{
		emit DevicesListChanged();
	}
	void OnBridgesChanged(QList<BridgeInfo> Bridges)
	{
		emit BridgesListChanged();
	}

	void PushScene(SceneInfo* in, int index);
	void AddScene();

private:
	QList< QSharedPointer<SceneInfo> > m_ScenesList;
};