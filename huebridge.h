#pragma once

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>
#include <QHostAddress>

#include <QThread>
#include <QMutex>

#include "ScreenCapture.h"

//-----------------------------------------------
struct EntertainmentMessage
{
      bool isXY;

      //isXY ? X : Red
      uint16_t R;

      //isXY ? Y : Green
      uint16_t G;

      //isXY ? Brightness : Blue
      uint16_t B;

      EntertainmentMessage()
          : R(0), G(0), B(0)
      {

      }

      EntertainmentMessage(uint16_t inR, uint16_t inG, uint16_t inB)
          : R(inR), G(inG), B(inB)
      {

      }
};

class EntertainmentCommThread : public QThread
{
    Q_OBJECT

public:
    explicit EntertainmentCommThread(QObject *parent, QString inUsername, QString inClientkey);

    void run() override;
    void threadsafe_setMessage(const EntertainmentMessage& inMessage);

private:
    QMutex messageMutex;
    EntertainmentMessage message;
    QString username;
    QString clientkey;
};
//-----------------------------------------------


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
    void entertainmentThreadFinished();

private:
	void setConnected(bool inConnected) {
		connected = inConnected;
		emit connectedChanged();
	}

    //path relative to http://address/api
    QNetworkRequest makeRequest(QString path, bool bIncludeUser = true);
    
    QString message;

    EntertainmentCommThread* eThread;

    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framegrabber;

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

protected:
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
    Q_PROPERTY(int x MEMBER x NOTIFY propertiesChanged)
    Q_PROPERTY(int y MEMBER y NOTIFY propertiesChanged)
    Q_PROPERTY(int z MEMBER z NOTIFY propertiesChanged)

signals:
    void propertiesChanged();
    
public:
    QString id;
    int x;
    int y;
    int z;

    explicit EntertainmentLight() : QObject(nullptr)
    {
    }

    explicit EntertainmentLight(QObject* parent, QString inId, int inX, int inY, int inZ) :
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
    Q_PROPERTY(QVector<EntertainmentLight> lights MEMBER lights NOTIFY propertiesChanged)

public:
    explicit EntertainmentGroup() : BridgeObject(nullptr)
    {
    }

    explicit EntertainmentGroup(HueBridge *parent) : BridgeObject(parent) 
    {
        isStreaming = false;
        emit propertiesChanged();
    }
    explicit EntertainmentGroup(const EntertainmentGroup& other) : BridgeObject(other.bridgeParent())
    {
        name = other.name;
        id = other.id;
        emit propertiesChanged();
    }
    EntertainmentGroup& operator=(const EntertainmentGroup& other)
    {
        setParent(other.parent());
        name = other.name;
        id = other.id;
        emit propertiesChanged();
        return *this;
    }

    void startStreaming();
    void stopStreaming();

    QString name;
    QVector<EntertainmentLight> lights;
    bool isStreaming;
};