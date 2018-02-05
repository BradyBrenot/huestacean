#include "bridgediscovery.h"
#include <QNetworkInterface>
#include <QHostAddress>

BridgeDiscovery::BridgeDiscovery(QObject *parent) 
    : QObject(parent)
    , hasSearched(false)
{
}

BridgeDiscovery::~BridgeDiscovery()
{
    emit closeSockets();
}

void BridgeDiscovery::startSearch()
{
    emit closeSockets();
    emit searchStarted();

    ////////////////////////////////////////////////
    //0a) this is the first search
    //   - Add known bridges (from QSettings)
    if (!hasSearched)
    {
        hasSearched = true;
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
            if (!addr.ip().protocol() == QUdpSocket::IPv4Protocol)
                continue;
            
            QUdpSocket* socket = new QUdpSocket(this);
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
}
void BridgeDiscovery::processPendingDatagrams()
{
    QUdpSocket *ssdpSocket = qobject_cast<QUdpSocket*>(sender());
    if (!ssdpSocket)
        return;

    QByteArray datagram;
    while (ssdpSocket->hasPendingDatagrams()) 
    {
        datagram.resize(int(ssdpSocket->pendingDatagramSize()));
        ssdpSocket->readDatagram(datagram.data(), datagram.size());
        if (datagram.contains("IpBridge"))
        {
            //Hue docs doth say: If the response contains “IpBridge”, it is considered to be a Hue bridge
            const int location = datagram.indexOf("http://");
            const int end = datagram.indexOf(":80", location);

            if (location == -1 || end == -1)
                return;

            addBridgeByIp(datagram.mid((location + 7), end - location - 7));
        }
    }
}

void BridgeDiscovery::saveBridges()
{

}

void BridgeDiscovery::addBridgeByIp(QString ipAddress)
{
    qDebug() << "addBridgeByIp" << ipAddress;

    //See if bridge already added
    foreach(HueBridge* bridge, bridges)
    {
        if (bridge->address == QHostAddress(ipAddress))
        {
            qDebug() << "already have that bridge, don't readd";
            return;
        }
    }

    HueBridgeSavedSettings Settings = HueBridgeSavedSettings(QHostAddress(ipAddress));
    HueBridge* bridge = new HueBridge(this, Settings);
    bridges.push_back(bridge);
    model.insert(bridge);
}