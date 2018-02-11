#pragma once

#include <QObject>
#include <QUdpSocket>
#include "huebridge.h"
#include "objectmodel.h"

class BridgeDiscovery : public QObject
{
    Q_OBJECT

    Q_PROPERTY(ObjectModel* model READ getModel NOTIFY modelChanged)

public:
    explicit BridgeDiscovery(QObject *parent = nullptr);
    virtual ~BridgeDiscovery();
    void saveBridges();
    ObjectModel* getModel() {
        return &model;
    }

public slots:
    void startSearch();
    void processPendingDatagrams();

signals:
    void modelChanged();
    void searchStarted();
    void searchCompleted();

    void closeSockets();

 private slots:
    void replied(QNetworkReply *reply);

private:
    void tryDescribeBridge(QString ipAddress);

    QVector<HueBridge*> bridges;
    ObjectModel model;
    bool hasSearched;

    QVector<HueBridgeSavedSettings> savedBridges;
};
