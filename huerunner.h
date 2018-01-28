#ifndef HUERUNNER_H
#define HUERUNNER_H

#include <QObject>
#include <QNetworkAccessManager>

class HueRunner : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)

public:
    explicit HueRunner(QObject *parent = nullptr);

    void setMessage(const QString &inMessage) {
        m_message = inMessage;
        qDebug() << "HueRunner:" << inMessage;
        emit messageChanged();
    }
    QString message() const {
        return m_message;
    }

    Q_INVOKABLE void connectToBridge();
    Q_INVOKABLE void resetConnection();

signals:
    void messageChanged();

public slots:

private slots:
    void replied(QNetworkReply *reply);

private:
    QString m_message;
    QNetworkAccessManager m_qnam;
};

#endif // HUERUNNER_H
