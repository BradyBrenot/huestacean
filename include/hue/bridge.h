#pragma once

#include "hue/hue.h"
#include "common/device.h"
#include "common/math.h"

#include <unordered_map>
#include <vector>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace Hue
{
	struct Streamer;

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

		//////////////////////////////////////////////////////////////////////////
		
		std::shared_ptr<Streamer> streamer;
		void StartFromUpdateThread(std::vector<DevicePtr> Lights);
		void Stop();
		void UpdateThreadCleanup();
		void Upload(const std::vector<std::tuple<uint32_t, Math::XyyColor>>& LightsToUpload);
		//////////////////////////////////////////////////////////////////////////

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

	signals:
		void WantsToggleStreaming(bool enable, int id, const std::vector<DevicePtr> Lights);

	private slots:
		void OnReplied(QNetworkReply* reply);
		void ToggleStreaming(bool enable, int id, const std::vector<DevicePtr>& Lights);

	private:
		std::shared_ptr<class QNetworkAccessManager> qnam;

		Status status;

		void NotifyListeners();
		std::unordered_map<int, std::function<void()>> listeners;

		//////////////////////////////////////////////////////////////////////////
		std::atomic_bool isStreamingEnabled;
		std::atomic_bool startingStreaming;
		std::atomic_int huestaceanGroupIndex;
	};
};