#pragma once

#include "hue/hue.h"
#include "common/device.h"

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
		virtual ~Bridge();

		void Connect(std::function<void()> callback);
		void RefreshDevices(std::function<void()> callback);

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

		std::vector<std::shared_ptr<Light>> Devices;

	private slots:
		void OnReplied(QNetworkReply* reply);

	private:
		std::shared_ptr<class QNetworkAccessManager> qnam;
		std::function<void()> connectCallback;
		std::function<void()> refreshCallback;
		Status status;
	};
};