#pragma once

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>

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
    explicit EntertainmentCommThread(QObject *parent = 0);

    void run() override;
    void threadsafe_setMessage(const EntertainmentMessage& inMessage);

private:
    QMutex messageMutex;
    EntertainmentMessage message;
};
//-----------------------------------------------


struct HueBridgeSavedSettings 
{
    QString ipAddress;
    QString userName;
    QString clientKey;

    HueBridgeSavedSettings::HueBridgeSavedSettings(QString inIpAddress, QString inUserName, QString inClientKey)
        : ipAddress(inIpAddress), userName(inUserName), clientKey(inClientKey)
    {}
};

/* Hue API wrapper */
class HueBridge : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ getMessage WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool connected READ getConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool manuallyAdded READ getManuallyAdded NOTIFY manuallyAddedChanged)

public:
    explicit HueBridge(QObject *parent = nullptr);
    explicit HueBridge(QObject *parent, bool bManuallyAdded);
    explicit HueBridge(QObject *parent, HueBridgeSavedSettings& SavedSettings, bool bReconnect = true);

    void setMessage(const QString &inMessage) {
        message = inMessage;
        qDebug() << "HueRunner:" << inMessage;
        emit messageChanged();
    }
    QString getMessage() const {
        return message;
    }

    bool getConnected() const {
        return connected;
    }

    bool getManuallyAdded() const {
        return manuallyAdded;
    }

    Q_INVOKABLE void connectToBridge();
    Q_INVOKABLE void resetConnection();

    Q_INVOKABLE void testEntertainment();

    void handleStreamingEnabled();

signals:
    void wantsLinkButton();

    //Property notifies
    void messageChanged();
    void connectedChanged();
    void manuallyAddedChanged();

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

    QString message;
    bool connected;
    bool manuallyAdded;

    QNetworkAccessManager qnam;

    EntertainmentCommThread* eThread;

    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framegrabber;
};