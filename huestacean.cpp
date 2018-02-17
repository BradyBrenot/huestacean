#include "huestacean.h"
#include "ScreenCapture.h"

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) 
    : QObject(parent),
    syncing(false)
{
    bridgeDiscovery = new BridgeDiscovery(this);
    bridgeDiscovery->startSearch();
    detectMonitors();

    connect(bridgeDiscovery, SIGNAL(modelChanged()),
        this, SLOT(connectBridges()));

    emit hueInit();
}

void Huestacean::detectMonitors()
{
    activeMonitorIndex = 0;

    std::vector<SL::Screen_Capture::Monitor> mons = SL::Screen_Capture::GetMonitors();
    foreach(QObject* monitor, monitors)
    {
        monitor->deleteLater();
    }
    monitors.clear();

    for (SL::Screen_Capture::Monitor& mon : mons)
    {
        Monitor* newMonitor = new Monitor(this, mon.Id, mon.Height, mon.Width, mon.OffsetX, mon.OffsetY, QString::fromUtf8(mon.Name, sizeof(mon.Name)), mon.Scaling);
        monitors.push_back(newMonitor);
    }

    emit monitorsChanged();
}

void Huestacean::startScreenSync()
{
    syncing = true;
    emit syncingChanged();
}
void Huestacean::stopScreenSync()
{
    syncing = false;
    emit syncingChanged();
}

void Huestacean::setActiveMonitor(int index)
{
    activeMonitorIndex = index;
}

void Huestacean::updateEntertainmentGroups()
{
    entertainmentGroups.clear();

    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            for (auto& group : bridge->EntertainmentGroups)
            {
                entertainmentGroups.push_back(&group);
            }
        }
    }

    emit entertainmentGroupsChanged();
}

void Huestacean::connectBridges()
{
    for (QObject* Obj : bridgeDiscovery->getModel())
    {
        HueBridge* bridge = qobject_cast<HueBridge*>(Obj);
        if (bridge != nullptr)
        {
            connect(bridge, SIGNAL(entertainmentGroupsChanged()),
                this, SLOT(updateEntertainmentGroups()));
            connect(bridge, SIGNAL(lightsChanged()),
                this, SIGNAL(entertainmentGroupsChanged()));
        }
    }
}