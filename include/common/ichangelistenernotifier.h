#pragma once

#include <set>
#include <vector>
#include <unordered_map>
#include <atomic>

class iChangeListenerNotifier
{
	struct _Listener
	{
		std::function<void()> callback;
		std::set<uint8_t> events;

		_Listener() : callback(nullptr), events() {

		}

		_Listener(std::function<void()> inCallback,
			std::set<uint8_t> inEvents) :
			callback(inCallback),
			events(inEvents)
		{

		}
	};
	
public:
	int RegisterListener(std::function<void()> callback, std::set<uint8_t> events = std::set<uint8_t>{})
	{
		int id = _nextListenerId++;
		_listeners[id] = _Listener{ callback, events };
		return id;
	}
	void UnregisterListener(int id)
	{
		_listeners.erase(id);
	}

	void NotifyListeners(int event = -1)
	{
		for (const auto& c : _listeners)
		{
			if (c.second.callback != nullptr
				&& (event == -1 || c.second.events.empty() || c.second.events.find(event) != c.second.events.end()))
			{
				c.second.callback();
			}
		}
	}

private:
	std::unordered_map<int, _Listener> _listeners;
	std::atomic_int _nextListenerId;
};
