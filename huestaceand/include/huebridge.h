#pragma once

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>
#include <QHostAddress>

#include <QThread>
#include <QMutex>


struct HueBridgeSavedSettings 
{
    QString id;
    QHostAddress address;
    QString userName;
    QString clientKey;

    HueBridgeSavedSettings()
        : id(), address(), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QHostAddress inAddress)
        : id(inId), address(inAddress), userName(), clientKey()
    {}

    HueBridgeSavedSettings(QString inId, QHostAddress inAddress, QString inUserName, QString inClientKey)
        : id(inId), address(inAddress), userName(inUserName), clientKey(inClientKey)
    {}
};

class Light;
class EntertainmentGroup;

/* Hue API wrapper */
class HueBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ getMessage WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool connected MEMBER connected NOTIFY connectedChanged)
    Q_PROPERTY(bool manuallyAdded MEMBER manuallyAdded NOTIFY onInit)
    Q_PROPERTY(QString address READ getAddress NOTIFY onInit)
    Q_PROPERTY(QString id MEMBER id NOTIFY onInit)
    Q_PROPERTY(QString friendlyName MEMBER friendlyName NOTIFY connectedChanged)
    Q_PROPERTY(bool wantsLinkButton MEMBER wantsLinkButton NOTIFY wantsLinkButtonChanged)

public:
    explicit HueBridge(QObject *parent, HueBridgeSavedSettings& SavedSettings, bool bManuallyAdded = false, bool bReconnect = true);

    void setMessage(const QString &inMessage) {
        message = inMessage;
        qDebug() << "HueRunner:" << inMessage;
        emit messageChanged();
    }
    QString getMessage() const {
        return message;
    }

    QString getAddress() const {
        return address.toString();
    }

    Q_INVOKABLE void connectToBridge();
    Q_INVOKABLE void resetConnection();

    void handleStreamingEnabled();

    QHash<QString, Light*> Lights;
    QHash<QString, EntertainmentGroup*> EntertainmentGroups;

    bool connected;
    bool manuallyAdded;
    bool wantsLinkButton;
    QHostAddress address;
    QString id;
    QString username;
    QString clientkey;
    QString friendlyName;

signals:
    //Property notifies
    void messageChanged();
    void connectedChanged();
    void streamingChanged();
    void wantsLinkButtonChanged();

    void entertainmentGroupsChanged();
    void lightsChanged();

    void onInit();

public slots:
    void requestGroups();

private slots:
    void replied(QNetworkReply *reply);

private:
	void setConnected(bool inConnected) {
		connected = inConnected;
		emit connectedChanged();
	}

    //path relative to http://address/api
    QNetworkRequest makeRequest(QString path, bool bIncludeUser = true);
    
    QString message;

    friend class Light;
    friend class EntertainmentGroup;
};

class BridgeObject : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString id MEMBER id NOTIFY propertiesChanged)

signals:
    void propertiesChanged();

public:
    QString id;
    explicit BridgeObject(HueBridge *parent) 
        : QObject(parent),
        id()
    {
    }
    HueBridge * bridgeParent() const { return reinterpret_cast<HueBridge*>(parent()); }
};

class Light : public BridgeObject
{
    Q_OBJECT;

    Q_PROPERTY(QString name MEMBER name NOTIFY propertiesChanged)

public:
    explicit Light() : BridgeObject(nullptr)
    {
    }

    explicit Light(HueBridge *parent) 
        : BridgeObject(parent),
        name()
    {
        emit propertiesChanged();
    }
    explicit Light(const Light& other) 
        : BridgeObject(other.bridgeParent())
    {
        name = other.name;
        emit propertiesChanged();
    }

    Light& operator=(const Light& other)
    {
        setParent(other.parent());
        name = other.name;
        id = other.id;
        emit propertiesChanged();
        return *this;
    }

    QString name;
};
