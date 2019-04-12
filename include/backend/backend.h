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
	void SetActiveRoom(int roomIndex);

	class RoomsWriter
	{
	public:
		RoomsWriter() = delete;
		RoomsWriter(const RoomsWriter& x) = delete;
		RoomsWriter(RoomsWriter&& x) = delete;
		~RoomsWriter() { b->roomsAreDirty = true; }

		explicit RoomsWriter(Backend* inBackend) : b(inBackend), lock(inBackend->roomsMutex)
		{

		}
		
		std::vector<Room>& GetRoomsMutable()
		{
			return b->rooms;
		};

	private:
		std::scoped_lock<std::shared_mutex> lock;
		Backend* b;
	};

	RoomsWriter GetRoomsWriter();

private:
	std::atomic_bool stopRequested;
	std::thread thread;

	std::shared_mutex roomsMutex;
	std::vector<Room> rooms;

	std::atomic_int activeRoomIndex;
	std::atomic_bool roomsAreDirty;

	std::unique_ptr<DeviceProvider> deviceProviders[ProviderType::Max];
};