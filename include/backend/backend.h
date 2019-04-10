#pragma once

#include <atomic>
#include <thread>
#include <shared_mutex>
#include <memory>
#include <vector>

#include "common/providertype.h"
#include "deviceprovider.h"
#include "common/room.h"

//Backend runs the flippin' show.

class Backend
{
public:
	Backend();
	~Backend();

	void Start();
	bool IsRunning();
	void Stop();

	std::vector<std::unique_ptr<class Room> > GetRooms();

private:
	std::atomic_bool stopRequested;
	std::thread thread;
	std::shared_mutex lock;

	std::vector<std::unique_ptr<Room> > rooms;
	std::vector<std::unique_ptr<DeviceProvider> > deviceProviders;
};