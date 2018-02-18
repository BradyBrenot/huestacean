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

    HueBridgeSavedSettings::HueBridgeSavedSettings()
        : id(), address(), userName(), clientKey()
    {}

    HueBridgeSavedSettings::HueBridgeSavedSettings(QString inId, QHostAddress inAddress)
        : id(inId), address(inAddress), userName(), clientKey()
    {}

    HueBridgeSavedSettings::HueBridgeSavedSettings(QString inId, QHostAddress inAddress, QString inUserName, QString inClientKey)
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

    QHash<QString, Light> Lights;
    QHash<QString, EntertainmentGroup> EntertainmentGroups;

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
    explicit BridgeObject(HueBridge *parent) : QObject(parent) {}
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

    explicit Light(HueBridge *parent) : BridgeObject(parent) 
    {
        emit propertiesChanged();
    }
    explicit Light(const Light& other) : BridgeObject(other.bridgeParent())
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

class EntertainmentLight : public QObject
{
    Q_OBJECT;

    Q_PROPERTY(QString id MEMBER id NOTIFY propertiesChanged)
    Q_PROPERTY(double x MEMBER x NOTIFY propertiesChanged)
    Q_PROPERTY(double y MEMBER y NOTIFY propertiesChanged)
    Q_PROPERTY(double z MEMBER z NOTIFY propertiesChanged)

signals:
    void propertiesChanged();
    
public:
    QString id;
    double x;
    double y;
    double z;

    explicit EntertainmentLight() : QObject(nullptr)
    {
    }

    explicit EntertainmentLight(QObject* parent, QString inId, double inX, double inY, double inZ) :
        QObject(parent), id(inId), x(inX), y(inY), z(inZ)
    {
        emit propertiesChanged();
    }

    explicit EntertainmentLight(const EntertainmentLight& other) : EntertainmentLight(other.parent(), other.id, other.x, other.y, other.z)
    {
    }
};

inline bool operator==(const EntertainmentLight& a, const EntertainmentLight& b)
{
    return a.id == b.id;
}

class EntertainmentGroup : public BridgeObject
{
    Q_OBJECT;

    Q_PROPERTY(QString name MEMBER name NOTIFY propertiesChanged)
    Q_PROPERTY(bool isStreaming MEMBER isStreaming NOTIFY propertiesChanged)
    Q_PROPERTY(QList<EntertainmentLight> lights MEMBER lights NOTIFY propertiesChanged)
    Q_PROPERTY(QString asString READ toString NOTIFY propertiesChanged)

public:
    explicit EntertainmentGroup() 
        : BridgeObject(nullptr),
        name(), lights(), isStreaming(false)
    {
    }

    explicit EntertainmentGroup(HueBridge *parent) 
        : BridgeObject(parent),
        name(), 
        lights(), 
        isStreaming(false)
    {
        emit propertiesChanged();
    }
    explicit EntertainmentGroup(const EntertainmentGroup& other) 
        : BridgeObject(other.bridgeParent()),
        name(other.name), 
        lights(other.lights), 
        isStreaming(false)
    {
        id = other.id;
        emit propertiesChanged();
    }
    EntertainmentGroup& operator=(const EntertainmentGroup& other)
    {
        setParent(other.parent());
        name = other.name;
        id = other.id;
        lights = other.lights;
        emit propertiesChanged();
        return *this;
    }

    QString toString() {
        return QString("%1 (%2)").arg(name, bridgeParent()->friendlyName);;
    }

    Q_INVOKABLE int numLights() { return lights.length(); }
    Q_INVOKABLE QObject* getLight(int index) { return &lights[index]; }
    Q_INVOKABLE void updateLightXZ(int index, float x, float z);

    void toggleStreaming(bool enable);

    QString name;
    QList<EntertainmentLight> lights;
    bool isStreaming;
};