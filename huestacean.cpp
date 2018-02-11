#include "huestacean.h"

QNetworkAccessManager qnam(nullptr);

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    bridgeDiscovery = new BridgeDiscovery(this);
    bridgeDiscovery->startSearch();
    emit hueInit();
}

void Huestacean::startScreenSync()
{

}