#include "LocalDataLogger.h"
using std::placeholders::_1;

/// @brief Creates a local data logger
/// @param RTC Pointer to RTC to use for time
LocalDataLogger::LocalDataLogger(ESP32Time* RTC) {
	rtc = RTC;
}

bool LocalDataLogger::begin() {
	// Set description
	Description.signalQuantity = 0;
	Description.type = "datalogger";
	Description.name = "Local Data Logger";
	Description.id = 1;
	bool result = false;
	if (!Storage::fileExists("/settings/sig/LocalLogger.json")) {
		if (!Storage::fileExists("/settings/sig")) {
			if (!Storage::fileExists("/settings")) {
				if (!Storage::createDir("/settings")) {
					return false;
				}
			}
			if (!Storage::createDir("/settings/sig")) {
					return false;
			}
		}
		// Set defaults
		current_config = { .name = "LocalData.csv", .enabled = false };
		path = "/data/" + current_config.name;
		result = saveConfig();
	} else {
		// Load settings
		result = setConfig(Storage::readFile("/settings/sig/LocalLogger.json"));
	}
	return result;
}

/// @brief Enables the local data logger
/// @param enable True to enable, false to disable 
/// @return True on success
bool LocalDataLogger::enableLogging(bool enable) {
	current_config.enabled = enable;
	if (enable) {
		// Check for existence of data file
		if (!Storage::fileExists(path)) {
			// Create file header
			header = "time";
			// Allocate the JSON document
			JsonDocument doc;
			// Deserialize sensor info
			DeserializationError error = deserializeJson(doc, SensorManager::getSensorInfo());			
			// Test if parsing succeeds.
			if (error) {
				Serial.print(F("Deserialization failed: "));
				Serial.println(error.f_str());
				return false;
			}
			for (const auto& s : doc["sensors"].as<JsonArray>()) {
				for (const auto& p : s["parameters"].as<JsonArray>()) {
					header += "," + p["name"].as<String>() + " (" + p["unit"].as<String>() + ")";
				}
			}
			header += '\n';
			if (!Storage::fileExists("/data")) {
				Storage::createDir("/data");
			}
			if (!Storage::writeFile(path, header)) {
				return false;
			}
		}
		return PeriodicTasks::addTask("LocalDataLogger", std::bind(&LocalDataLogger::logData, this));
	} else {
		return PeriodicTasks::removeTask("LocalDataLogger");
	}
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool LocalDataLogger::setConfig(String config) {
	// Allocate the JSON document
  	JsonDocument doc;
	// Deserialize file contents
	DeserializationError error = deserializeJson(doc, config);
	// Test if parsing succeeds.
	if (error) {
		Serial.print(F("Deserialization failed: "));
		Serial.println(error.f_str());
		return false;
	}
	// Assign loaded values
	current_config.name = doc["name"].as<String>();
	current_config.enabled = doc["enabled"].as<bool>();
	path = "/data/" + current_config.name;
	if (!saveConfig()) {
		return false;
	}
	enableLogging(current_config.enabled);
	return true;
}

/// @brief Saves the current config to a JSON file
/// @return True on success
bool LocalDataLogger::saveConfig() {
	return Storage::writeFile("/settings/sig/LocalLogger.json", getConfig());
}

/// @brief Logs current data from all sensors
void LocalDataLogger::logData() {
	if (current_config.enabled) {
		if (!Storage::fileExists(path)) {
			if (!Storage::writeFile(path, header)) {
				return;
			}
		}
		if (!SensorManager::takeMeasurement()) {
			return;
		}
		String data = rtc->getTime("%m-%d-%Y %T");
		// Allocate the JSON document
		JsonDocument doc;
		// Deserialize sensor info
		DeserializationError error = deserializeJson(doc, SensorManager::getLastMeasurement());
		// Test if parsing succeeds.
		if (error) {
			Serial.print(F("Deserialization failed: "));
			Serial.println(error.f_str());
			return;
		}
		for (const auto& m : doc["measurements"].as<JsonArray>()) {
			data += "," + m["value"].as<String>();
		}
		data += '\n';
		if (Storage::freeSpace() > data.length()) {
			Storage::appendToFile(path, data);
		}
	}
}

/// @brief Gets the current config
/// @return A JSON string of the config
String LocalDataLogger::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["name"] = current_config.name;
	doc["enabled"] = current_config.enabled;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}