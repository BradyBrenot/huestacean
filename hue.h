#ifndef HUERUNNER_H
#define HUERUNNER_H

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
    void threadsafe_setMessage(const EntertainmentMessage& message);

private:
    QMutex m_messageMutex;
    EntertainmentMessage m_message;
};
//-----------------------------------------------




/* Hue API wrapper */
class Hue : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)
    Q_PROPERTY(bool connected READ connected WRITE setConnected NOTIFY connectedChanged)

public:
    explicit Hue(QObject *parent = nullptr);

    void setMessage(const QString &inMessage) {
        m_message = inMessage;
        qDebug() << "HueRunner:" << inMessage;
        emit messageChanged();
    }
    QString message() const {
        return m_message;
    }

    void setConnected(bool inConnected) {
        m_connected = inConnected;
        emit connectedChanged();
    }
    bool connected() const {
        return m_connected;
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

public slots:
    void requestGroups();

private slots:
    void replied(QNetworkReply *reply);
    void entertainmentThreadFinished();

private:
    QString m_message;
    bool m_connected;

    QNetworkAccessManager m_qnam;

    EntertainmentCommThread* m_eThread;

    std::shared_ptr<SL::Screen_Capture::IScreenCaptureManager> framegrabber;

};

#endif // HUERUNNER_H
