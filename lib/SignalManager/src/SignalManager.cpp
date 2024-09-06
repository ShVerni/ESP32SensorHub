#include "SignalManager.h"

/// @brief Creates a SignalManager object
SignalManager::SignalManager() {
	// Create the signal queue
	signalQueue = xQueueCreate(15, sizeof(int[2]));
}

/// @brief Adds a signal receiver to the in-use list
/// @param receiver A pointer to the receiver to add
/// @return True on success
bool SignalManager::addReceiver(SignalReceiver* receiver) {
	// Add receiver to in-use list
	receivers.push_back(receiver_info {
		.positionID = (int)receivers.size(),
		.receiver = receiver
	});
	return true; // Currently no way to fail this
}

/// @brief Calls the begin function on all the in-use signal receivers
/// @return True if all receivers started correctly
bool SignalManager::beginReceivers() {
	for (auto const &r : receivers) {
		if (!r.receiver->begin()) {
			Serial.println("Could not start " + r.receiver->Description.name);
			return false;
		} else {
			Serial.println("Started " + r.receiver->Description.name);
		}
	}
	return true;
}


/// @brief Adds a signal to the queue for processing
/// @param receiverPosID The position ID of the signal receiver
/// @param signal The name of the signal
/// @param payload An optional JSON string for data payload
/// @return True on success
bool SignalManager::addSignalToQueue(int receiverPosID, String signal, String payload) {
	// Check if receiver is in-use
	if(receiverPosID < 0 || receiverPosID >= receivers.size()) {
		Serial.println("Receiver position Id out of range");
		return false;
	}

	// Attempt to convert signal name to ID
	int signal_id;
	try {
		signal_id = receivers[receiverPosID].receiver->Description.signals.at("signal");
	} catch (const std::out_of_range& e) {
		Serial.println("Receiver cannot process signal");
		return false;
	}

	// Attempt to add signal to queue
	return addSignalToQueue(receiverPosID, signal_id, payload);
}

/// @brief Adds a signal to the queue for processing
/// @param receiverPosID The position ID of the signal receiver
/// @param signal The ID of the signal
/// @param payload An optional JSON string for data payload
/// @return True on success
bool SignalManager::addSignalToQueue(int receiverPosID, int signal, String payload) {
	// Check if receiver is in-use
	if(receiverPosID < 0 || receiverPosID >= receivers.size()) {
		Serial.println("Receiver position Id out of range");
		return false;
	}

	// Create signal array for queue
	int new_signal[] { receiverPosID, signal };

	// Add signal array to queue
	if (xQueueSend(signalQueue, &new_signal, 10) != pdTRUE) {
		Serial.println("Signal queue full");
		return false;
	}
	// Add payload to queue
	payloads.push(payload);
	return true;
}

/// @brief Retrieves the information on all available receivers and their signals
/// @return A JSON string of the information
String SignalManager::getReceiverInfo() {
	// Allocate the JSON document
	JsonDocument doc;
	// Create array of receivers
	JsonArray receiver_array = doc["receivers"].to<JsonArray>();

	for (auto const &r : receivers) {
		// Add receiver description to JSON document 
		receiver_array[r.positionID]["positionID"] = r.positionID;
		receiver_array[r.positionID]["description"]["signalQuantity"] = r.receiver->Description.signalQuantity;
		receiver_array[r.positionID]["description"]["type"] = r.receiver->Description.type;
		receiver_array[r.positionID]["description"]["name"] = r.receiver->Description.name;
		receiver_array[r.positionID]["description"]["id"] = r.receiver->Description.id;
		// Add signals and IDs to JSON document
		for (auto const &s : r.receiver->Description.signals) {
			receiver_array[r.positionID]["signals"][s.second] = s.first;
		}
	}
	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}


/// @brief Gets any available config settings for a signal receive device
/// @param receiverPosID The position ID of the signal receive
/// @return A JSON string of configurable settings
String SignalManager::getReceiverConfig(int receiverPosID) {
	return receivers[receiverPosID].receiver->getConfig();
}

/// @brief Gets any available config settings for a signal receiver device
/// @param receiverPosID The position ID of the signal receive
/// @param config A JSON string of the configuration
/// @return True on success
bool SignalManager::setReceiverConfig(int receiverPosID, String config) {
	return receivers[receiverPosID].receiver->setConfig(config);
}

/// @brief Executes a signal on a receiver immediately. Use carefully, may cause issues with signals also being processed from queue
/// @param receiverPosID The position ID of the signal receiver
/// @param signal The name of the signal
/// @param payload An optional JSON string for data payload
/// @return A JSON string that is the response from the signal
String SignalManager::processSignalImmediately(int receiverPosID, String signal, String payload) {
	// Check if receiver is in-use
	if(receiverPosID < 0 || receiverPosID >= receivers.size()) {
		Serial.println("Receiver position Id out of range");
		return R"({"success": false})";
	}

	// Attempt to convert signal name to ID
	int signal_id;
	try {
		signal_id = receivers[receiverPosID].receiver->Description.signals.at("signal");
	} catch (const std::out_of_range& e) {
		Serial.println("Receiver cannot process signal");
		return R"({"success": false})";
	}
	// Process signal
	return processSignalImmediately(receiverPosID, signal_id, payload);
}

/// @brief Executes a signal on a receiver immediately. Use carefully, may cause issues with signals also being processed from queue
/// @param receiverPosID The position ID of the signal receiver
/// @param signal The ID of the signal
/// @param payload An optional JSON string for data payload
/// @return A JSON string that is the response from the signal
String SignalManager::processSignalImmediately(int receiverPosID, int signal, String payload) {
	// Check if receiver is in-use
	if(receiverPosID < 0 || receiverPosID >= receivers.size()) {
		Serial.println("Receiver position Id out of range");
		return R"({"success": false})";
	}
	// Process signal
	return receivers[receiverPosID].receiver->receiveSignal(signal, payload);
}

/// @brief Wraps the signal processor task for static access
/// @param signalManager Pointer to the SignalManger object
void SignalManager::SignalProcessorTaskWrapper(void* signalManager) {
	static_cast<SignalManager*>(signalManager)->processSignal();
}

/// @brief Processes signals from queue
void SignalManager::processSignal() {
	int signal[2];
	while(true) {
		if (xQueueReceive(signalQueue, &signal, 10) == pdTRUE)
		{
			receivers[signal[0]].receiver->receiveSignal(signal[1], payloads.front());
			payloads.pop();
		}
	}
}