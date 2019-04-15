#include "hue/bridgediscovery.h"
#include "hue/hue.h"

#include <QNetworkInterface>
#include <QHostAddress>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

using namespace Hue;

BridgeDiscovery::BridgeDiscovery(std::shared_ptr<class QNetworkAccessManager> inQnam)
	: QObject(nullptr)
	, qnam(inQnam)
	, hasSearched(false)
{
	connect(qnam.get(), SIGNAL(finished(QNetworkReply*)),
		this, SLOT(Replied(QNetworkReply*)));
}

BridgeDiscovery::~BridgeDiscovery()
{
	emit closeSockets();
}

void BridgeDiscovery::Search(std::vector<std::string> manualAddresses, bool doScan, std::function<void(std::vector<Bridge> const&)> callback)
{
	emit closeSockets();
	emit searchStarted();

	CallSearchCallback();
	searchCallback = callback;

	////////////////////////////////////////////////
	//0a) this is the first search
	//   - Add known bridges (from QSettings)
	if (!hasSearched)
	{
		hasSearched = true;

		QSettings settings;
		settings.beginGroup("BridgeDiscovery");

		int size = settings.beginReadArray("bridges");
		for (int i = 0; i < size; ++i) {
			settings.setArrayIndex(i);

			const QString id = settings.value("id").toString();
			const QString addressStr = settings.value("address").toString();
			const QHostAddress address = QHostAddress(addressStr);
			const QString username = settings.value("username").toString();
			const QString clientkey = settings.value("clientkey").toString();

			TryDescribeBridge(addressStr);
		}
		settings.endArray();

		settings.endGroup();
	}
	//0b) this is not the first search
	//   - Remove all non-connected non-known bridges for the re-search
	else
	{

	}

	////////////////////////////////////////////////
	//1. SSDP UPNP discovery
	char data[] =
		"M-SEARCH * HTTP/1.1\r\n"
		"HOST: 239.255.255.250:1900\r\n"
		"ST: ssdp:all\r\n"
		"MAN: \"ssdp:discover\"\r\n"
		"MX: 1\r\n"
		"\r\n"
		;

	// Go through every interface and every address on the system and listen/send on each (AnyIPv4 doesn't do anything)
	foreach(QNetworkInterface iface, QNetworkInterface::allInterfaces())
	{
		if (!(iface.flags() & QNetworkInterface::CanMulticast))
			continue;

		foreach(QNetworkAddressEntry addr, iface.addressEntries())
		{
			if (addr.ip().protocol() != QUdpSocket::IPv4Protocol)
				continue;

			QUdpSocket * socket = new QUdpSocket(this);
			QObject::connect(socket, SIGNAL(readyRead()), SLOT(processPendingDatagrams()));
			QObject::connect(this, SIGNAL(closeSockets()), socket, SLOT(deleteLater()));
			if (socket->bind(addr.ip(), 0, QUdpSocket::ShareAddress))
			{
				socket->joinMulticastGroup(QHostAddress("239.255.255.250"));
				socket->writeDatagram(data, sizeof(data), QHostAddress("239.255.255.250"), 1900);
			}
			else
			{
				socket->deleteLater();
			}
		}
	}

	//TODO: 
	//2. N-UPNP
	//3. IP scan
	//4. Manually-entered
}
void BridgeDiscovery::processPendingDatagrams()
{
	QUdpSocket* ssdpSocket = qobject_cast<QUdpSocket*>(sender());
	if (!ssdpSocket)
		return;

	QByteArray datagram;
	while (ssdpSocket->hasPendingDatagrams())
	{
		datagram.resize(int(ssdpSocket->pendingDatagramSize()));
		ssdpSocket->readDatagram(datagram.data(), datagram.size());
		if (datagram.contains("IpBridge"))
		{
			//Hue docs doth say: If the response contains IpBridge, it is considered to be a Hue bridge
			const int start = datagram.indexOf("http://");
			const int end = datagram.indexOf(":80", start);

			if (start == -1 || end == -1)
			{
				qWarning() << "Bad reply from IpBridge:" << datagram;
			}

			QMetaObject::invokeMethod(this, "TryDescribeBridge", Qt::QueuedConnection, Q_ARG(QString, datagram.mid((start + 7), end - start - 7)));
		}
	}
}

void BridgeDiscovery::TryDescribeBridge(QString ipAddress)
{
	//See if bridge already added
	for(const Bridge& bridge : bridges)
	{
		if (bridge.address == QHostAddress(ipAddress).toIPv4Address())
		{
			return;
		}
	}

	QNetworkRequest r = QNetworkRequest(QUrl(QString("http://%1/description.xml").arg(ipAddress)));
	r.setOriginatingObject(this);
	qnam->get(r);
}

void BridgeDiscovery::Replied(QNetworkReply * reply)
{
	if (reply->request().originatingObject() != this)
		return;

	reply->deleteLater();

	qDebug() << "BridgeDiscovery got describe reply for" << reply->request().url().toString();

	QByteArray data = reply->readAll();

	const int start = data.indexOf("<serialNumber>");
	const int end = data.indexOf("</serialNumber>", start);

	if (start == -1 || end == -1)
	{
		qWarning() << "Bad reply from bridge" << data;
		return;
	}

	const QString id = data.mid((start + 14), end - start - 14);
	const QString url = reply->request().url().toString();
	const QString ipAddress = url.mid(url.indexOf("http://") + 7, url.indexOf("/description.xml") - url.indexOf("http://") - 7);

	for(const auto& bridge : bridges)
	{
		if (bridge.address == QHostAddress(ipAddress).toIPv4Address()
			|| bridge.id == id.toUtf8().constData())
		{
			qDebug() << "already have that bridge, don't readd";

			return;
		}
	}

	qDebug() << "bridge that replied was" << id << "at" << ipAddress;

	auto bridge = Bridge(qnam, id.toUtf8().constData(), QHostAddress(ipAddress).toIPv4Address());

	bridge.SetStatus(Bridge::Status::Discovered);

	bridges.push_back(bridge);

	CallSearchCallback();
}

void BridgeDiscovery::CallSearchCallback()
{
	if (searchCallback == nullptr) {
		return;
	}

	searchCallback(bridges);
}