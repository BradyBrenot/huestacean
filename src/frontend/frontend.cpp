#include "frontend/frontend.h"

#include <QSignalBlocker>

Frontend::Frontend(std::shared_ptr<Backend> inBackend) 
	: FrontendSimpleSource(nullptr)
	, m_Backend(inBackend)
{
	//Set state based on Backend

	//Listen for changes to Backend and update accordingly
	//

	//Connect to listen to self changes
	/*void ScenesChanged(QList<SceneInfo> Scenes);
    void DevicesChanged(QList<DeviceInfo> Devices);
    void BridgesChanged(QList<BridgeInfo> Bridges);
    void RazerChanged(QList<RazerInfo> Razer);*/
}