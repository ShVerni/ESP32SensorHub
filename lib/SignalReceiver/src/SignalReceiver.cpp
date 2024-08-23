#include "SignalReceiver.h"

/// @brief Sets up the signal receiver
/// @return True on success
bool SignalReceiver::begin() {
	return false;
}

/// @brief Receives and reacts to a signal
/// @param signal The signal ID number to react to
/// @param payload An optional JSON string for data payload
/// @return A JSON response, if any. May be ignored by caller, but must include a boolean success element
String SignalReceiver::receiveSignal(int signal, String payload) {
	if (signal >= 0 && signal < Description.signalQuantity)
		return R"({"success": false})";
	else 
		return R"({"success": true})";
}