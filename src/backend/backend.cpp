#include "backend/backend.h"
#include "common/lightupdate.h"

#include <chrono>
#include <thread>
#include <algorithm>

using namespace std::chrono_literals;

Backend::Backend() :
	stopRequested(false),
	thread(),
	roomsMutex(),
	rooms(),
	activeRoomIndex(0),
	roomsAreDirty(false),
	deviceProviders()
{
}

Backend::~Backend()
{
	Stop();
}

void Backend::Start()
{
	//DOES NOT mutate rooms
	//DOES NOT read from rooms without locking roomsMutex

	if (IsRunning()) {
		return;
	}

	stopRequested = false;
	thread = std::thread([this] {
		Room renderRoom = rooms.size() > activeRoomIndex ? rooms[activeRoomIndex] : Room();

		LightUpdateParams lightUpdate;
		std::vector<HsluvColor> Colors;
		std::vector<Box> Positions;
		std::vector<Device> Devices;

		auto tick = [&](float deltaTime)
		{
			//Copy the new room if necessary
			if (roomsAreDirty)
			{
				{
					std::scoped_lock lock(roomsMutex);
					int updateThreadRoomIndex = activeRoomIndex;
					renderRoom = rooms.size() > updateThreadRoomIndex ? rooms[updateThreadRoomIndex] : Room();
				}

				//Update and dirty Positions and Devices
				lightUpdate.colorsDirty = true;
				lightUpdate.positionsDirty = true;
				lightUpdate.devicesDirty = true;

				//Query every device to fetch positions off it
				//Fill in big dumb non-sparse Devices array
				//Blank out Colors

				//@TODO
			}

			//Run effects
			for (auto& effect : renderRoom.effects)
			{
				effect->Tick(deltaTime);
				effect->Update(Positions, Colors);
			}
			lightUpdate.colorsDirty = true;

			//Send light data to device providers

			//@TODO

			//DONE
		};

		auto lastStart = std::chrono::high_resolution_clock::now();

		while (!stopRequested) {
			constexpr auto tickRate = 16.67ms;

			auto start = std::chrono::high_resolution_clock::now();
			float deltaTime = std::chrono::duration<float>{ start - lastStart }.count();
			lastStart = start;

			tick(deltaTime);

			auto end = std::chrono::high_resolution_clock::now();

			//sleep to keep our tick rate right, or at least 1ms
			auto timeLeft = tickRate - (end - start);
			auto sleepForTime = timeLeft > 1ms ? timeLeft : 1ms;
			std::this_thread::sleep_for(sleepForTime);
		}
	});
}

bool Backend::IsRunning()
{
	return thread.joinable();
}

void Backend::Stop()
{
	if (!IsRunning()) {
		return;
	}

	stopRequested = true;
	thread.join();
}

const std::vector<Room>& Backend::GetRooms() const
{
	return rooms;
}

Backend::RoomsWriter Backend::GetRoomsWriter()
{
	return RoomsWriter(this);
}