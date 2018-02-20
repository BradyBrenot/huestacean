#pragma once

#include <QObject>
#include <QUdpSocket>
#include "huebridge.h"
#include "objectmodel.h"

class BridgeDiscovery : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QList<QObject*> model READ getModel NOTIFY modelChanged)

public:
    explicit BridgeDiscovery(QObject *parent = nullptr);
    virtual ~BridgeDiscovery();
    void saveBridges();

    QList<QObject*> getModel() {
        return *reinterpret_cast<const QList<QObject*>*>(&bridges);
    }

    Q_INVOKABLE void manuallyAddIp(QString ipAddress) {
        tryDescribeBridge(ipAddress);
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

    QList<HueBridge*> bridges;
    bool hasSearched;

    QVector<HueBridgeSavedSettings> savedBridges;
};
