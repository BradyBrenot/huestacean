#include "backend/backend.h"

#include <chrono>
#include <thread>
#include <algorithm>

using namespace std::chrono_literals;

Backend::Backend() :
	stopRequested(false),
	thread(),
	lock(),
	rooms(),
	deviceProviders()
{

}

Backend::~Backend()
{
	Stop();
}

void Backend::Start()
{
	if (thread.joinable()) {
		return;
	}

	stopRequested = false;
	thread = std::thread([&] {
		while (!stopRequested) {
			constexpr auto tickRate = 16.67ms;

			auto start = std::chrono::high_resolution_clock::now();
			// ...
			auto end = std::chrono::high_resolution_clock::now();

			//sleep to keep our tick rate right, or at least 1ms
			auto timeLeft = tickRate - (end - start);
			auto sleepForTime = timeLeft > 1ms ? timeLeft : 1ms;
			std::this_thread::sleep_for(sleepForTime);
		}
	});
}

bool Backend::IsThreadRunning()
{
	return thread.joinable();
}

void Backend::Stop()
{
	if (!IsThreadRunning()) {
		return;
	}

	stopRequested = true;
	thread.join();
}