#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSettings>

#include "huerunner.h"

QString SETTING_USERNAME = "Bridge/Username";
QString SETTING_CLIENTKEY = "Bridge/clientkey";

HueRunner::HueRunner(QObject *parent) : QObject(parent)
{
    connect(&m_qnam, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replied(QNetworkReply*)));
    setMessage("Not connected.");
}

void HueRunner::connectToBridge()
{
    QSettings settings;
    if(settings.contains(SETTING_USERNAME) && settings.contains(SETTING_CLIENTKEY))
    {
        //Verify existing registration
        QNetworkRequest qnr(QUrl(QString("http://192.168.0.102/api/%1/config").arg(settings.value("username").toString())));
    }
    else
    {
        //Register
        QNetworkRequest qnr(QUrl("http://192.168.0.102/api"));
        qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject json;
        json.insert("devicetype","huestacean#windows");
        json.insert("generateclientkey",true);

        m_qnam.post(qnr, QJsonDocument(json).toJson());
    }
}
void HueRunner::resetConnection()
{
    QSettings settings;
    settings.remove(SETTING_USERNAME);
    settings.remove(SETTING_CLIENTKEY);
}

void HueRunner::replied(QNetworkReply *reply)
{
    reply->deleteLater();

    if(reply->request().url().toString().endsWith("/api"))
    {
        QSettings settings;
        QByteArray data = reply->readAll();

        qDebug() << "We got: \n" << QString::fromUtf8(data.data());

        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        if(!replyJson.isArray() || replyJson.array().size() == 0)
        {
            setMessage("Bad reply from bridge");
            return;
        }

        QJsonObject obj = replyJson.array()[0].toObject();
        if(obj.contains(QString("success")))
        {
            //Connected!
            QString username = obj["success"].toObject()["username"].toString();
            QString clientkey = obj["success"].toObject()["clientkey"].toString();

            qDebug() << "Registered with bridge. Username:" << username << "Clientkey:" << clientkey;

            settings.setValue(SETTING_USERNAME, username);
            settings.setValue(SETTING_CLIENTKEY, clientkey);

            setMessage("Connected to bridge!");
        }
        else
        {
            if(obj[QString("error")].toObject()[QString("type")].toInt() == 101)
            {
                setMessage("Press the link button!");
            }
        }
    }
    else if(reply->request().url().toString().endsWith("/api"))
    {

    }
    else
    {
        qCritical() << "Received reply to unknown request" << reply->request().url();
    }
}
