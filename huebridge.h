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
class Group;

/* Hue API wrapper */
class HueBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ getMessage WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool connected MEMBER connected NOTIFY connectedChanged)
    Q_PROPERTY(bool manuallyAdded MEMBER manuallyAdded NOTIFY onInit)
    Q_PROPERTY(QHostAddress address MEMBER address NOTIFY onInit)
    Q_PROPERTY(bool wantsLinkButton MEMBER wantsLinkButton NOTIFY streamingChanged)

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

    Q_INVOKABLE void connectToBridge();
    Q_INVOKABLE void resetConnection();

    void handleStreamingEnabled();

    QHash<QString, Light*> Lights;
    QHash<QString, Group*> Groups;

    bool connected;
    bool manuallyAdded;
    bool wantsLinkButton;
    QHostAddress address;
    QString id;
    QString username;
    QString clientkey;

signals:
    //Property notifies
    void messageChanged();
    void connectedChanged();
    void groupsChanged();
    void lightsChanged();
    void streamingChanged();
    void wantsLinkButtonChanged();

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
    friend class Group;
};

class BridgeObject : public QObject
{
    Q_OBJECT;

public:
    QString id;
    explicit BridgeObject(HueBridge *parent) : QObject(parent) {}

protected:
    HueBridge * bridgeParent() { return reinterpret_cast<HueBridge*>(parent()); }
};

class Light : public BridgeObject
{
    Q_OBJECT;

public:
    explicit Light(HueBridge *parent) : BridgeObject(parent) {}
};

class Group : public BridgeObject
{
    Q_OBJECT;

public:
    explicit Group(HueBridge *parent) : BridgeObject(parent) {}

    void startStreaming();
};