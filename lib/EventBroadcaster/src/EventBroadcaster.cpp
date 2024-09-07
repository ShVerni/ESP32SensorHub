#include "EventBroadcaster.h"

/// @brief Begins on the subscribed receivers
/// @return True on success
bool EventBroadcaster::beginReceivers() {
	for (const auto& r : receivers) {
		if (!r->begin()) {
			return false;
		}
	}
	return true;
}

/// @brief Broadcasts an event to all subscribed receivers
/// @param event The event to broadcast
/// @return True on success
bool EventBroadcaster::broadcastEvent(Events event) {
	for (const auto& r : receivers) {
		if (!r->receiveEvent((int)event)) {
			return false;
		}
	}
	return true;
}

/// @brief Subscribes a receiver to the events
/// @param receiver A pointer to the receiver
/// @return True on success
bool EventBroadcaster::addReceiver(EventReceiver* receiver) {
	receivers.push_back(receiver);
	return true; // Currently no way to fail this
}