#pragma once

#include <QObject>
#include <QUdpSocket>
#include <QNetworkReply>
#include "hue/hue.h"
#include "hue/bridge.h"

//@TODO: remove BridgeDiscovery inheritence of QObject
// This is mostly taken from older pre-rewrite code.
// I don't mind using Qt networking libraries because they're multiplatform
// and I'm already linking against Qt, but would like to remove any Qt C++
// extensions from my "backend" headers / keep it to a minimum in source files

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