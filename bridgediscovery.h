#pragma once

#include <QObject>
#include <QUdpSocket>
#include "huebridge.h"
#include "objectmodel.h"

class BridgeDiscovery : public QObject
{
    Q_OBJECT

public:
    explicit BridgeDiscovery(QObject *parent = nullptr);
    virtual ~BridgeDiscovery();
    void saveBridges();

public slots:
    void startSearch();
    void processPendingDatagrams();

signals:
    void searchStarted();
    void searchCompleted();

    void closeSockets();

private:
    void addBridgeByIp(QString ipAddress);

    QVector<HueBridge*> bridges;
    ObjectModel model;
    bool hasSearched;
};
