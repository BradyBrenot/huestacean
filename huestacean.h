#pragma once

#include <QObject>
#include "huebridge.h"
#include "objectmodel.h"
#include "bridgediscovery.h"

class Huestacean : public QObject
{
    Q_OBJECT

    Q_PROPERTY(HueBridge* hueBridge READ getHueBridge NOTIFY hueInit)
    Q_PROPERTY(BridgeDiscovery* bridgeDiscovery READ getBridgeDiscovery NOTIFY hueInit)

public:
    explicit Huestacean(QObject *parent = nullptr);

    HueBridge* getHueBridge() {
        return hueBridge;
    }
    BridgeDiscovery* getBridgeDiscovery() {
        return bridgeDiscovery;
    }

    //temporary home of screen sync
    Q_INVOKABLE void startScreenSync();

signals:
    void hueInit();

private:
	HueBridge* hueBridge;
    BridgeDiscovery* bridgeDiscovery;
};