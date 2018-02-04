#include "huestacean.h"

Huestacean::Huestacean(QObject *parent) : QObject(parent)
{
    hueBridge = new HueBridge;
    bridgeDiscovery = new BridgeDiscovery;
    emit hueInit();
}
