#include <QNetworkReply>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QThread>
#include <QDateTime>
#include <QMetaMethod>

#include "huebridge.h"
#include "huestacean.h"
#include "entertainment.h"

QString SETTING_USERNAME = "Bridge/Username";
QString SETTING_CLIENTKEY = "Bridge/clientkey";

HueBridge::HueBridge(QObject *parent, HueBridgeSavedSettings& SavedSettings, bool bManuallyAdded/* = false*/, bool bReconnect/* = true*/)
    : QObject(parent),
    address(SavedSettings.address),
    id(SavedSettings.id),
    username(SavedSettings.userName),
    clientkey(SavedSettings.clientKey),
    manuallyAdded(bManuallyAdded),
    wantsLinkButton(false)
{
    connect(&qnam, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replied(QNetworkReply*)));

    connect(this, SIGNAL(connectedChanged()),
        this, SLOT(requestGroups()));

    setMessage("Not connected.");
    setConnected(false);

    if (bReconnect)
    {
        metaObject()->method(metaObject()->indexOfMethod("connectToBridge()")).invoke(this, Qt::QueuedConnection);
    }
}

QNetworkRequest HueBridge::makeRequest(QString path, bool bIncludeUser/* = true*/)
{
    if (bIncludeUser)
    {
        QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api/%2%3").arg(address.toString(), username, path)));
        request.setOriginatingObject(this);
        return request;
    }
    else
    {
        QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api%2").arg(address.toString(), path)));
        request.setOriginatingObject(this);
        return request;
    }
}

void HueBridge::connectToBridge()
{
    setMessage("Connecting.");
    setConnected(false);

    if (!username.isEmpty() && !clientkey.isEmpty())
    {
        //Verify existing registration
        QNetworkRequest qnr = makeRequest("/config");
        qnam.get(qnr);
    }
    else
    {
        //Register
        QNetworkRequest qnr = makeRequest("", false);
        qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject json;
        json.insert("devicetype", "huestacean#windows");
        json.insert("generateclientkey", true);

        qnam.post(qnr, QJsonDocument(json).toJson());
    }
}
void HueBridge::resetConnection()
{
    username = QString();
    clientkey = QString();
}
void HueBridge::requestGroups()
{
    QNetworkRequest qnr = makeRequest("/lights");
    qnam.get(qnr);

    qnr = makeRequest("/groups");
    qnam.get(qnr);
}

void HueBridge::replied(QNetworkReply *reply)
{
    if (reply->request().originatingObject() != this)
        return;

    reply->deleteLater();

    if (reply->request().url().toString().endsWith("/api"))
    {
        QByteArray data = reply->readAll();

        qDebug() << "We got: \n" << QString::fromUtf8(data.data());

        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        if (!replyJson.isArray() || replyJson.array().size() == 0)
        {
            setMessage("Bad reply from bridge");
            return;
        }

        QJsonObject obj = replyJson.array()[0].toObject();
        if (obj.contains(QString("success")))
        {
            //Connected!
            username = obj["success"].toObject()["username"].toString();
            clientkey = obj["success"].toObject()["clientkey"].toString();

            qDebug() << "Registered with bridge. Username:" << username << "Clientkey:" << clientkey;

            setMessage("Registered and connected to bridge!");

            setConnected(true);
        }
        else
        {
            if (obj[QString("error")].toObject()[QString("type")].toInt() == 101)
            {
                setMessage("Press the link button!");

                wantsLinkButton = true;
                emit wantsLinkButtonChanged();
            }
        }
    }
    else if (reply->request().url().toString().endsWith("/config"))
    {
        QByteArray data = reply->readAll();

        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        if (!replyJson.isObject() || !replyJson.object().contains("whitelist"))
        {
            qDebug() << "Connection failed" << replyJson.isObject() << replyJson.object().contains("whitelist");
            resetConnection();
        }
        else
        {
            setMessage("Connected! Reused old connection!");
            qDebug() << "Reply" << replyJson;
            friendlyName = replyJson.object()["name"].toString();
            setConnected(true);
        }
    }
    else if (reply->request().url().toString().endsWith("/lights"))
    {
        QByteArray data = reply->readAll();
        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        QJsonObject obj = replyJson.object();

        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            QString string = it.key();
            Light& newLight = Lights[it.key()] = Light(this);
            newLight.id = it.key();
            newLight.name = it.value().toObject()["name"].toString();
        }

        emit lightsChanged();
    }
    else if (reply->request().url().toString().endsWith("/groups"))
    {
        QByteArray data = reply->readAll();
        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        QJsonObject obj = replyJson.object();

        for (auto it = obj.begin(); it != obj.end(); ++it)
        {
            if (it.value().toObject()["type"].toString().compare(QString("entertainment"), Qt::CaseInsensitive) == 0)
            {
                QString string = it.key();
                EntertainmentGroup& newGroup = EntertainmentGroups[it.key()] = EntertainmentGroup(this);
                newGroup.id = it.key();
                newGroup.name = it.value().toObject()["name"].toString();
                QJsonObject locations = it.value().toObject()["locations"].toObject();

                for (auto j = locations.begin(); j != locations.end(); ++j)
                {
                    QJsonArray loc = j.value().toArray();
                    newGroup.lights.push_back(EntertainmentLight(this, j.key(), loc[0].toDouble(), loc[1].toDouble(), loc[2].toDouble()));
                }
            }                        
        }

        emit entertainmentGroupsChanged();
    }
    else if (false /* handle activation of an entertainment group here? */)
    {
        
    }
    else
    {
        qCritical() << "Received reply to unknown request" << reply->request().url();

        QByteArray data = reply->readAll();
        QJsonDocument replyJson = QJsonDocument::fromJson(data);
        qDebug().noquote() << replyJson.toJson(QJsonDocument::Indented);
    }
}
