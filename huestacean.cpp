#include "huestacean.h"

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    bridgeDiscovery = new BridgeDiscovery;
    emit hueInit();
}

void Huestacean::startScreenSync()
{

}