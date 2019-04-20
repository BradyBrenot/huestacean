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
	huestaceanGroupIndex(-1)
{
	connect(qnam.get(), SIGNAL(finished(QNetworkReply*)),
		this, SLOT(OnReplied(QNetworkReply*)));

	connect(this, SIGNAL(WantsToggleStreaming(bool, int, const std::vector<DevicePtr>)),
		this, SLOT(ToggleStreaming(bool, int, const std::vector<DevicePtr>)));
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

void Bridge::RefreshDevices()
{
	QNetworkRequest qnr = MakeRequest(*this, "/lights");
	qnam->get(qnr);

	qnr = MakeRequest(*this, "/groups");
	qnam->get(qnr);
}

static std::atomic_int nextListenerId;

int Bridge::RegisterListener(std::function<void()> callback)
{
	int id = nextListenerId++;
	listeners[id] = callback;
	return id;
}
void Bridge::UnregisterListener(int id)
{
	listeners.erase(id);
}

void Bridge::ToggleStreaming(bool enable, int id, const std::vector<DevicePtr>& Lights)
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
			arr.push_back(static_cast<int>(light->id));
		}

		body.insert("lights", arr);
	}

	qnam->put(qnr, QJsonDocument(body).toJson());
}

void Bridge::StartFromUpdateThread(std::vector<DevicePtr> Lights)
{
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
				l->id = id;
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

			NotifyListeners();
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

		NotifyListeners();
	}
#endif
	else if (reply->request().url().toString().endsWith("/groups"))
	{
		QByteArray data = reply->readAll();
		if (data.contains("success"))
		{
			QJsonDocument replyJson = QJsonDocument::fromJson(data);
			auto foundId = replyJson.array()[0].toObject()["id"].toInt();
		}
		else
		{
			QByteArray data = reply->readAll();
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

			if (huestaceanGroupIndex == -1)
			{
				//Create a new group
				QNetworkRequest qnr = MakeRequest(*this, "/groups", false);
				qnr.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

				QJsonObject json;
				json.insert("name", "_huestacean");
				json.insert("type", "entertainment");

				qnam->post(qnr, QJsonDocument(json).toJson());

				//@TODO -- will this succeed with an empty group?
			}
		}
	}
	else if (reply->request().url().toString().contains("/groups/"))
	{
		QByteArray data = reply->readAll();
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
	NotifyListeners();

	if (s == Bridge::Status::Connected)
	{
		RefreshDevices();
	}
}

void Bridge::NotifyListeners()
{
	for (const auto& c : listeners)
	{
		if (c.second != nullptr)
		{
			c.second();
		}
	}
}