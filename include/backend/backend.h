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

	std::vector<Room>& GetRooms() const;
	std::vector<Room>& GetRoomsMutable();
	void SetActiveRoom(int roomIndex);

private:
	std::atomic_bool stopRequested;
	std::thread thread;

	std::shared_mutex roomsMutex;
	std::vector<Room> rooms;

	std::atomic_int activeRoomIndex;
	std::atomic_bool roomsAreDirty;

	std::unique_ptr<DeviceProvider> deviceProviders[ProviderType::Max];
};