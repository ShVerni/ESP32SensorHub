#include "Sensor.h"

/// @brief Used to run initial setup and configure a sensor
/// @return True on success
bool Sensor::begin() {
	return false;
}

/// @brief Has as sensor take a measurement
/// @return True on success
bool Sensor::takeMeasurement() {
	return false;
}

/// @brief Used to calibrate sensor
/// @param step The calibration step to execute for multi-step calibration processes
/// @return A tuple with the fist element as a Sensor::calibration_response and the second an optional message String accompanying the response
std::tuple<Sensor::calibration_response, String> Sensor::calibrate(int step) {
	return { Sensor::calibration_response::error, "No calibration method" };
}

/// @brief Gets any available config settings for the current device
/// @return A JSON string of configurable settings
String Sensor::getConfig() {
	return "{}";
}

/// @brief Updates configuration of device
/// @param config A JSON string of the configuration settings
/// @return True on success
bool Sensor::setConfig(String config) {
	return true;
}