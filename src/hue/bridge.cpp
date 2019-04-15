#include "hue/bridge.h"

#include <QNetworkReply>
#include <QHostAddress>

#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDateTime>

using namespace Hue;

Bridge::Bridge(std::shared_ptr<QNetworkAccessManager> inQnam, std::string inId, uint32_t inAddress)
	: QObject(nullptr),
	id(inId),
	address(inAddress),
	qnam(inQnam),
	status(Status::Undiscovered),
	username(),
	clientkey()
{
	connect(qnam.get(), SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replied(QNetworkReply*)));

	connect(this, SIGNAL(connectedChanged()),
		this, SLOT(requestGroups()));
}

Bridge::Bridge(const Bridge& b)
	: Bridge(b.qnam, b.id, b.address)
{
	status = b.status;
	username = b.username;
	clientkey = b.clientkey;
}

//path relative to http://address/api
QNetworkRequest MakeRequest(Bridge& bridge, QString path, bool bIncludeUser = true)
{
	if (bIncludeUser)
	{
		QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api/%2%3").arg(QHostAddress(bridge.address).toString(), QString(bridge.username.c_str()), path)));
		request.setOriginatingObject(&bridge);
		return request;
	}
	else
	{
		QNetworkRequest request = QNetworkRequest(QUrl(QString("http://%1/api%2").arg(QHostAddress(bridge.address).toString(), path)));
		request.setOriginatingObject(&bridge);
		return request;
	}
}

void Bridge::Connect(std::function<void()> callback)
{
	connectCallback = callback;

	if (status == Status::Connected)
	{
		status = Status::Discovered;
		SetStatus(Status::Connected);
	}

	if (!username.empty() && !clientkey.empty())
	{
		//Verify existing registration
		QNetworkRequest qnr = MakeRequest(*this, "/config");
		qnam->get(qnr);
	}
	else
	{
		//Register
		QNetworkRequest qnr = MakeRequest(*this, "", false);
		qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

		QJsonObject json;
		json.insert("devicetype", "huestacean#windows");
		json.insert("generateclientkey", true);

		qnam->post(qnr, QJsonDocument(json).toJson());
	}
}

void Bridge::RefreshDevices(std::function<void()> callback)
{
	refreshCallback = callback;

	QNetworkRequest qnr = MakeRequest(*this, "/lights");
	qnam->get(qnr);

	qnr = MakeRequest(*this, "/groups");
	qnam->get(qnr);
}

void Bridge::OnReplied(QNetworkReply* reply)
{
	// this is all pretty old, hasn't had to change since I first did it
	//@TODO: error handling? Right now it just ignores bad / unexpected replies

	if (reply->request().originatingObject() != this)
		return;

	reply->deleteLater();

	if (reply->request().url().toString().endsWith("/api"))
	{
		QByteArray data = reply->readAll();

		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		if (!replyJson.isArray() || replyJson.array().size() == 0)
		{
			qDebug() << "Bad reply from bridge";
			return;
		}

		QJsonObject obj = replyJson.array()[0].toObject();
		if (obj.contains(QString("success")))
		{
			//Connected!
			username = obj["success"].toObject()["username"].toString().toUtf8();
			clientkey = obj["success"].toObject()["clientkey"].toString().toUtf8();

			qDebug() << "Registered with bridge. Username:" << obj["success"].toObject()["username"].toString() << "Clientkey:" << obj["success"].toObject()["clientkey"].toString();

			SetStatus(Status::Connected);
		}
		else
		{
			if (obj[QString("error")].toObject()[QString("type")].toInt() == 101)
			{
				SetStatus(Status::WantsLink);
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
			clientkey = "";
			username = "";

			SetStatus(Bridge::Status::WantsLink);
		}
		else
		{
			qDebug() << "Connected! Reused old connection!";
			friendlyName = replyJson.object()["name"].toString().toUtf8();
			SetStatus(Status::Connected);
		}
	}
	else if (reply->request().url().toString().endsWith("/lights"))
	{
		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		QJsonObject obj = replyJson.object();

		for (auto it = obj.begin(); it != obj.end(); ++it)
		{
			bool ok;
			auto id = it.key().toUInt(&ok);

			if (!ok) {
				qDebug() << "failed to parse light id at" << reply->request().url().toString() << "--id--" << id;
				return;
			}

			if (it.value().isObject()) {
				qDebug() << "unexpected reply type from" << reply->request().url().toString();
				return;
			}

			auto jsonObj = it.value().toObject();

			auto name = std::string(jsonObj["name"].toString().toUtf8());
			auto type = std::string(jsonObj["type"].toString().toUtf8());
			auto productname = std::string(jsonObj["productname"].toString().toUtf8());
			auto uniqueid = std::string(jsonObj["uniqueid"].toString().toUtf8());

			auto reachable = jsonObj["state"].isObject() ? jsonObj["state"].toObject()["reachable"].toBool() : false;

			//Find a matching device in our existing devices array
			bool foundMatch = false;

			auto setProps = [&](std::shared_ptr<Light>& l)
			{
				l->id = id;
				l->uniqueid = uniqueid;
				l->name = name;
				l->type = type;
				l->productname = productname;
				l->reachable = reachable;
				l->setIsConnected(reachable);
			};

			for (std::shared_ptr<Light> l : Devices)
			{
				if (uniqueid != "")
				{
					//We can match perfectly!
					foundMatch = l->uniqueid == uniqueid;
				}
				else
				{
					//take a guess. id might never change, I don't really know, this should be better
					foundMatch = uniqueid == l->uniqueid
						&& name == l->name
						&& type == l->type;
				}

				if (foundMatch)
				{
					setProps(l);
					break;
				}
			}

			if (!foundMatch)
			{
				Devices.push_back(std::make_shared<Light>());
				setProps(Devices.back());
			}
			//MakeRequest(*this, QString("/lights/%1").arg(id));

			if (refreshCallback != nullptr) {
				refreshCallback();
			}
		}
	}
#if 0
	else if (reply->request().url().toString().contains("/lights/"))
	{
		auto s = reply->request().url().toString();
		auto list = s.split('/', QString::SkipEmptyParts);

		if (list.size() == 0) {
			qDebug() << "failed to fetch light info at" << reply->request().url().toString();
			return;
		}

		bool ok;
		auto id = list.last().toInt(&ok);

		if (!ok) {
			qDebug() << "failed to fetch light info at" << reply->request().url().toString() << "--id--" << id;
			return;
		}

		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		if (!replyJson.isObject()) {
			qDebug() << "unexpected reply type from" << reply->request().url().toString();
			return;
		}

		auto jsonObj = replyJson.object();

		if(!replyJson.object().contains()

		if (refreshCallback != nullptr) {
			refreshCallback();
		}
	}
#endif
	else if (reply->request().url().toString().endsWith("/groups"))
	{
		//@TODO process entertainment groups
#if 0
		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		QJsonObject obj = replyJson.object();

		for (auto it = obj.begin(); it != obj.end(); ++it)
		{
			if (it.value().toObject()["type"].toString().compare(QString("entertainment"), Qt::CaseInsensitive) == 0)
			{
				QString string = it.key();
				EntertainmentGroup* newGroup = EntertainmentGroups[it.key()] = new EntertainmentGroup(this);
				newGroup->id = it.key();
				newGroup->name = it.value().toObject()["name"].toString();
				QJsonObject locations = it.value().toObject()["locations"].toObject();

				for (auto j = locations.begin(); j != locations.end(); ++j)
				{
					QJsonArray loc = j.value().toArray();
					newGroup->lights.push_back(EntertainmentLight(this, j.key(), loc[0].toDouble(), loc[1].toDouble(), loc[2].toDouble()));
				}
			}
		}

		emit entertainmentGroupsChanged();
#endif
	}
	else if (false /* handle activation of an entertainment group here? */)
	{

	}
	else
	{
		qWarning() << "Received reply to unknown request" << reply->request().url();

		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
	}
}

Bridge::Status Bridge::GetStatus()
{
	return status;
}

void Bridge::SetStatus(Bridge::Status s)
{
	if (connectCallback != nullptr)
	{
		connectCallback();
	}
}
