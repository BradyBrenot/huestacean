#pragma once

#include "rep_frontend.h"
#include "backend/backend.h"

#include <memory>

class Frontend : public FrontendSimpleSource
{
	Q_OBJECT

public:
	explicit Frontend(std::shared_ptr<Backend> inBackend);

public:
	virtual ~Frontend() {}


private:
	void BackendActiveSceneChanged();
	void BackendScenesChanged();

	void BackendHueChanged();
	void BackendRazerChanged();

	void BackendDevicesChanged();

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

	void RemoteActiveSceneIndexChanged(qint32 ActiveSceneIndex);
	void RemoteScenesChanged(QList<SceneInfo> Scenes);
	void RemoteDevicesChanged(QList<DeviceInfo> Devices);
	void RemoteBridgesChanged(QList<BridgeInfo> Bridges);
	void RemoteRazerChanged(QList<RazerInfo> Razer);
};