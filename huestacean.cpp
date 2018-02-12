#include "huestacean.h"
#include "ScreenCapture.h"

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    bridgeDiscovery = new BridgeDiscovery(this);
    bridgeDiscovery->startSearch();
    detectMonitors();

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

}

void Huestacean::setActiveMonitor(int index)
{
    activeMonitorIndex = index;
}