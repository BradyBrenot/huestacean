#include "huestacean.h"

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    bridgeDiscovery = new BridgeDiscovery(this);
    emit hueInit();
}

void Huestacean::startScreenSync()
{

}