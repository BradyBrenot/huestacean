#pragma once

#include "hue/hue.h"
#include "common/device.h"

#include <unordered_map>
#include <vector>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace Hue
{
	class Bridge : public QObject
	{
	Q_OBJECT

	public:
		Bridge(std::shared_ptr<QNetworkAccessManager> inQnam, std::string inId, uint32_t inAddress);
		Bridge(const Bridge& b);
		Bridge& operator=(const Bridge& b);

		void Connect();
		void RefreshDevices();

		int RegisterListener(std::function<void()> callback);
		void UnregisterListener(int id);

		std::string id;
		uint32_t address;

		std::string username;
		std::string clientkey;
		std::string friendlyName;

		enum class Status : uint8_t
		{
			Undiscovered,
			Discovered,
			WantsLink,
			Connected
		};
		Status GetStatus();
		void SetStatus(Status s);

		std::vector<std::shared_ptr<Light>> devices;

	private slots:
		void OnReplied(QNetworkReply* reply);

	private:
		std::shared_ptr<class QNetworkAccessManager> qnam;

		Status status;

		void NotifyListeners();
		std::unordered_map<int, std::function<void()>> listeners;
	};
};