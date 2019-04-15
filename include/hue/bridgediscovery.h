#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QNetworkReply>
#include "hue/hue.h"
#include "hue/bridge.h"

namespace Hue
{
	class BridgeDiscovery : public QObject
	{
		Q_OBJECT

	public:
		BridgeDiscovery(std::shared_ptr<class QNetworkAccessManager> inQnam);
		virtual ~BridgeDiscovery();

		void Search(std::vector<std::string> manualAddresses, bool doScan, std::function<void(std::vector<Bridge> const&)> callback);

	public slots:
		void processPendingDatagrams();

	signals:
		void searchStarted();
		void searchCompleted();

		void closeSockets();

	private slots:
		void Replied(QNetworkReply* reply);
		void TryDescribeBridge(QString ipAddress);

	private:
		std::vector<Bridge> bridges;
		std::function<void(std::vector<Bridge> const&)> searchCallback;
		bool hasSearched;
		std::shared_ptr<class QNetworkAccessManager> qnam;

		void CallSearchCallback();
	};
}