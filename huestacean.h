#pragma once

#include <QObject>
#include <QStringListModel>
#include "huebridge.h"
#include "objectmodel.h"
#include "bridgediscovery.h"

extern QNetworkAccessManager qnam;

class Monitor : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int id MEMBER id NOTIFY propertiesChanged)
    Q_PROPERTY(int height MEMBER height NOTIFY propertiesChanged)
    Q_PROPERTY(int width MEMBER width NOTIFY propertiesChanged)
    Q_PROPERTY(int offsetX MEMBER offsetX NOTIFY propertiesChanged)
    Q_PROPERTY(int offsetY MEMBER offsetY NOTIFY propertiesChanged)
    Q_PROPERTY(QString name MEMBER name NOTIFY propertiesChanged)
    Q_PROPERTY(float scaling MEMBER scaling NOTIFY propertiesChanged)
    Q_PROPERTY(QString asString READ toString NOTIFY propertiesChanged)

public:
    explicit Monitor(QObject *parent = nullptr) 
        : QObject(parent)
        , id(INT32_MAX), height(0), width(0), offsetX(0), offsetY(0), name(), scaling(1.0f)
    {
        emit propertiesChanged();
    }

    explicit Monitor(QObject *parent, int inId, int inHeight, int inWidth, int inOffsetX, int inOffsetY, QString inName, float inScaling)
        : QObject(parent)
        , id(inId), height(inHeight), width(inWidth), offsetX(inOffsetX), offsetY(inOffsetY), name(inName), scaling(inScaling)
    {
        emit propertiesChanged();
    }

    QString toString() {
        return QString("%2 (%3x%4)").arg(QString::number(id), QString::number(width), QString::number(height));
    }

    int id;
    int height;
    int width;
    int offsetX;
    int offsetY;
    QString name;
    float scaling;

signals:
    void propertiesChanged();
};

class Huestacean : public QObject
{
    Q_OBJECT

    Q_PROPERTY(HueBridge* hueBridge READ getHueBridge NOTIFY hueInit)
    Q_PROPERTY(BridgeDiscovery* bridgeDiscovery READ getBridgeDiscovery NOTIFY hueInit)
    Q_PROPERTY(QList<QObject*> monitorsModel READ getMonitorsModel NOTIFY monitorsChanged)

public:
    explicit Huestacean(QObject *parent = nullptr);

    HueBridge* getHueBridge() {
        return hueBridge;
    }
    BridgeDiscovery* getBridgeDiscovery() {
        return bridgeDiscovery;
    }
    QList<QObject*> getMonitorsModel() {
        return monitors;
    }

signals:
    void hueInit();

private:
	HueBridge* hueBridge;
    BridgeDiscovery* bridgeDiscovery;

    QList<QObject*> monitors;

    /////////////////////////////////////////////////////////////////////////////
    // temporary home of screen sync
public:
    Q_INVOKABLE void detectMonitors();
    Q_INVOKABLE void startScreenSync();
signals:
    void monitorsChanged();

public slots:
    void setActiveMonitor(int index);

private:
    int activeMonitorIndex;
};