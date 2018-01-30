#ifndef HUERUNNER_H
#define HUERUNNER_H

#include <QObject>
#include <QDebug>

#include <QNetworkAccessManager>

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

private:
    QString m_message;
    bool m_connected;

    QNetworkAccessManager m_qnam;
};

#endif // HUERUNNER_H
