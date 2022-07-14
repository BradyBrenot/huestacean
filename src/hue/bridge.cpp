#include "hue/bridge.h"
#include "hue/streamer.h"

#include <atomic>

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
	clientkey(),
	isStreamingEnabled(false),
	startingStreaming(false),
	huestaceanGroupIndex(-1)
{
	connect(qnam.get(), SIGNAL(finished(QNetworkReply*)),
		this, SLOT(OnReplied(QNetworkReply*)));

	qRegisterMetaType<std::vector<DevicePtr>>("std::vector<DevicePtr>");

	connect(this, SIGNAL(WantsToggleStreaming(bool, int, std::vector<DevicePtr>)),
		this, SLOT(ToggleStreaming(bool, int, std::vector<DevicePtr>)));
}

Bridge::Bridge(const Bridge& b)
	: Bridge(b.qnam, b.id, b.address)
{
	status = b.status;
	username = b.username;
	clientkey = b.clientkey;
	huestaceanGroupIndex = int{ b.huestaceanGroupIndex };
}

Bridge& Bridge::operator=(const Bridge& b)
{
	qnam = b.qnam;
	id = b.id;
	address = b.address;
	status = b.status;
	username = b.username;
	clientkey = b.clientkey;

	return *this;
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

void Bridge::Connect()
{
	if (status == Status::Connected)
	{
		status = Status::Discovered;
		SetStatus(Status::Connected);
		return;
	}

	qDebug() << "trying to connect to bridge" << id.c_str();

	if (!username.empty() && !clientkey.empty())
	{
		qDebug() << "bridge" << id.c_str() << "has existing registration" << username.c_str() << clientkey.c_str();

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

void Bridge::RefreshDevices()
{
	qDebug() << id.c_str() << "requesting lights and groups";
	QNetworkRequest qnr = MakeRequest(*this, "/lights");
	qnam->get(qnr);
}

void Bridge::RefreshGroups()
{
	QNetworkRequest qnr = MakeRequest(*this, "/groups");
	qnam->get(qnr);
}

void Bridge::ToggleStreaming(bool enable, int id, std::vector<DevicePtr> Lights)
{
	QNetworkRequest qnr = MakeRequest(*this, QString("/groups/%1").arg(id));
	qnr.setOriginatingObject(this);
	qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

	QJsonObject stream;
	stream.insert("active", enable);

	QJsonObject body;
	body.insert("stream", stream);

	if (enable)
	{
		QJsonArray arr;
		for (const auto& l : Lights)
		{
			auto* light = dynamic_cast<Light*>(l.get());
			arr.push_back(QString::number(light->id));
		}

		body.insert("lights", arr);
	}

	qnam->put(qnr, QJsonDocument(body).toJson());
}

void Bridge::StartFromUpdateThread(std::vector<DevicePtr> Lights)
{
	UpdateThreadLastLights = Lights;

	if (isStreamingEnabled
		|| huestaceanGroupIndex == -1
		|| startingStreaming) {
		return;
	}

	startingStreaming = true;
	emit WantsToggleStreaming(true, huestaceanGroupIndex, Lights);
}
void Bridge::Stop()
{
	if (!isStreamingEnabled) {
		return;
	}

	emit WantsToggleStreaming(false, huestaceanGroupIndex, std::vector<DevicePtr>());
	isStreamingEnabled = false;
}

void Bridge::UpdateThreadCleanup()
{
	streamer = nullptr;
}

void Bridge::Upload(const std::vector<std::tuple<uint32_t, Math::XyyColor>> & LightsToUpload)
{
	if (!isStreamingEnabled)
	{
		StartFromUpdateThread(UpdateThreadLastLights);
		return;
	}

	if (!streamer
		|| !streamer->isValid)
	{
		streamer = std::make_shared<Streamer>(*this);
	}

	streamer->Upload(LightsToUpload);
}

void Bridge::OnReplied(QNetworkReply* reply)
{
	// this is all pretty old, hasn't had to change since I first did it
	//@TODO: error handling? Right now it just ignores bad / unexpected replies

	qDebug() << "Got reply to" << reply->request().url().toString();

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
			username = std::string(obj["success"].toObject()["username"].toString().toUtf8());
			clientkey = std::string(obj["success"].toObject()["clientkey"].toString().toUtf8());

			qDebug() << "Registered with bridge. Username:" << obj["success"].toObject()["username"].toString() << "Clientkey:" << obj["success"].toObject()["clientkey"].toString();

			//Fetch config so we can ge the friendlyname
			QNetworkRequest qnr = MakeRequest(*this, "/config");
			qnam->get(qnr);

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

		//qDebug() << "/config replied" << data;

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
			qDebug() << "Connected and signed in!";
			friendlyName = std::string(replyJson.object()["name"].toString().toUtf8());
			SetStatus(Status::Connected);
		}
	}
	else if (reply->request().url().toString().endsWith("/lights"))
	{
		qDebug() << "got reply to /lights";

		QByteArray data = reply->readAll();
		QJsonDocument replyJson = QJsonDocument::fromJson(data);
		QJsonObject obj = replyJson.object();

		for (auto it = obj.begin(); it != obj.end(); ++it)
		{
			bool ok;
			auto foundLightId = it.key().toUInt(&ok);

			if (!ok) {
				qDebug() << "failed to parse light id at" << reply->request().url().toString() << "--id--" << foundLightId;
				return;
			}

			if (!it.value().isObject()) {
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
				l->id = foundLightId;
				l->uniqueid = uniqueid;
				l->bridgeid = id;
				l->name = name;
				l->type = type;
				l->productname = productname;
				l->reachable = reachable;
				l->setIsConnected(reachable);
			};

			for (std::shared_ptr<Light>& l : devices)
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
				devices.push_back(std::make_shared<Light>());
				setProps(devices.back());
			}
			//MakeRequest(*this, QString("/lights/%1").arg(id));

			NotifyListeners(EVENT_DEVICES_CHANGED);
		}

		RefreshGroups();
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

		NotifyListeners();
	}
#endif
	else if (reply->request().url().toString().endsWith("/groups"))
	{
		QByteArray data = reply->readAll();

		qDebug() << "Received reply to /groups" << data;

		if (data.contains("success"))
		{
			QJsonDocument replyJson = QJsonDocument::fromJson(data);

			huestaceanGroupIndex = replyJson.array()[0].toObject()["success"].toObject()["id"].toString().toInt();
		}
		else
		{
			QJsonDocument replyJson = QJsonDocument::fromJson(data);
			QJsonObject obj = replyJson.object();

			for (auto it = obj.begin(); it != obj.end(); ++it)
			{
				if (it.value().toObject()["name"].toString().compare(QString("_huestacean"), Qt::CaseInsensitive) == 0)
				{
					QString idString = it.key();
					huestaceanGroupIndex = idString.toInt();
					break;
				}
			}

			if (huestaceanGroupIndex == -1
				&& devices.size() > 0)
			{
				//Create a new group
				QNetworkRequest qnr = MakeRequest(*this, "/groups");
				qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

				QJsonObject json;
				json.insert("name", "_huestacean");
				json.insert("type", "Entertainment");
				json.insert("class", "TV");

				QJsonArray arr;
				auto* light = dynamic_cast<Light*>(devices[0].get());
				arr.push_back(QString::number(light->id));

				json.insert("lights", arr);

				qnam->post(qnr, QJsonDocument(json).toJson());
			}
		}
	}
	else if (reply->request().url().toString().contains("/groups/"))
	{
		QByteArray data = reply->readAll();

		qDebug() << "Received reply to /groups/" << data;

		isStreamingEnabled = data.contains("true");
		startingStreaming = false;
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
	status = s;
	NotifyListeners(EVENT_STATUS_CHANGED);

	switch (s)
	{
	case Bridge::Status::Connected:
		qDebug() << "Connected!";
		break;
	case Bridge::Status::WantsLink:
		qDebug() << "WantsLink!";
		break;
	case Bridge::Status::Discovered:
		qDebug() << "Discovered!";
		break;
	case Bridge::Status::Undiscovered:
		qDebug() << "Undiscovered!";
		break;

	}

	if (s == Bridge::Status::Connected)
	{
		RefreshDevices();
	}
}