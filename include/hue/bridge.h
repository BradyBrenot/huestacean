#pragma once

#include "hue/hue.h"
#include "common/device.h"
#include "common/math.h"
#include "common/changelistenernotifier.h"

#include <unordered_map>
#include <vector>
#include <QObject>

class QNetworkAccessManager;
class QNetworkReply;

namespace Hue
{
	struct Streamer;

	class Bridge : public QObject, public ChangeListenerNotifier
	{
		Q_OBJECT

	public:
		Bridge(std::shared_ptr<QNetworkAccessManager> inQnam, std::string inId, uint32_t inAddress);
		Bridge(const Bridge& b);
		Bridge& operator=(const Bridge& b);

		//////////////////////////////////////////////////////////////////////////

		const int EVENT_STATUS_CHANGED = 1;
		const int EVENT_DEVICES_CHANGED = 2;

		//////////////////////////////////////////////////////////////////////////

		void Connect();
		void RefreshDevices();
		void RefreshGroups();

		//////////////////////////////////////////////////////////////////////////
		
		std::shared_ptr<Streamer> streamer;
		void StartFromUpdateThread(std::vector<DevicePtr> Lights);
		std::vector<DevicePtr> UpdateThreadLastLights;
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
		void WantsToggleStreaming(bool enable, int id, std::vector<DevicePtr> Lights);

	private slots:
		void OnReplied(QNetworkReply* reply);
		void ToggleStreaming(bool enable, int id, std::vector<DevicePtr> Lights);

	private:
		std::shared_ptr<class QNetworkAccessManager> qnam;

		Status status;

		//////////////////////////////////////////////////////////////////////////
		std::atomic_bool isStreamingEnabled;
		std::atomic_bool startingStreaming;
		std::atomic_int huestaceanGroupIndex;
	};
};