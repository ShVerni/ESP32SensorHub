#include "LocalDataLogger.h"

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
	if (!Storage::fileExists(config_path)) {
		// Set defaults
		current_config = { .name = "LocalData.csv", .enabled = false };
		TaskDescription = { .taskName = "LocalDataLogger", .taskPeriod = 10000 };
		path = "/data/" + current_config.name;
		result = saveConfig(config_path, getConfig());
	} else {
		// Load settings
		result = setConfig(Storage::readFile(config_path));
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
	}
	return enableTask(enable);
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
	TaskDescription.taskPeriod = doc["samplingPeriod"].as<long>();
	TaskDescription.taskName = doc["taskName"].as<std::string>();
	path = "/data/" + current_config.name;
	enableLogging(current_config.enabled);
	return saveConfig(config_path, getConfig());
}

/// @brief Logs current data from all sensors
/// @param elapsed The time in ms since this task was last called
void LocalDataLogger::runTask(long elapsed) {
	totalElapsed += elapsed;
	if (current_config.enabled && totalElapsed >= TaskDescription.taskPeriod) {
		totalElapsed = 0;
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
	doc["samplingPeriod"] = TaskDescription.taskPeriod;
	doc["taskName"] = TaskDescription.taskName;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}