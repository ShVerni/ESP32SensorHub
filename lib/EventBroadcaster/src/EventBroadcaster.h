/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* Contributors: Sam Groveman
*/

#pragma once
#include <vector>
#include <EventReceiver.h>
	
/// @brief Broadcasts device events
class EventBroadcaster {
	private:
		/// @brief Stores all event receivers
		std::vector<EventReceiver*> receivers;

	public:
		/// @brief Stores possible events to raise
		enum Events { Clear, Starting, Ready, Updating, Rebooting, Running, WifiConfig, Error };

		bool beginReceivers();
		bool broadcastEvent(Events event);
		bool addReceiver(EventReceiver* receiver);

};
