#include "TimerSwitch.h"

/// @brief Creates a new TimerSwitch
/// @param RTC A pointer to a ESP32Time object to use
/// @param Pin Pin to use
/// @param configFile The name of the config file to use
TimerSwitch::TimerSwitch(ESP32Time* RTC, int Pin, String configFile) : GenericOutput(Pin, configFile) {
	rtc = RTC;
}

/// @brief Starts a timer switch 
/// @return True on success
bool TimerSwitch::begin() {
	// Set description
	Description.signalQuantity = 1;
	Description.type = "output";
	Description.name = "Timer Switch";
	Description.signals = {{"state", 0}};
	Description.id = 0;
	bool result = false;
	TaskDescription = { .taskName = "Timer Switch", .taskPeriod = 30000 };
	// Create settings directory if necessary
	if (!checkConfig(config_path)) {
		// Set defaults
		return setConfig(R"({"pin":)" + String(current_config.pin) + R"(, "name": "Timer Switch", "onTime": "9:30", "offTime": "22:15", "enabled": false, "active": "Active high"})");
	} else {
		// Load settings
		return setConfig(Storage::readFile(config_path));
	}
}

/// @brief Gets the current config
/// @return A JSON string of the config
String TimerSwitch::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["pin"] = current_config.pin;
	doc["name"] = current_config.name;
	doc["onTime"] = current_config.onTime;
	doc["offTime"] = current_config.offTime;
	doc["enabled"] = current_config.enabled;
	doc["active"]["current"] =  current_config.active;
	doc["active"]["options"][0] = "Active low";
	doc["active"]["options"][1] = "Active high";

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool TimerSwitch::setConfig(String config) {
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
	// Disable task in case name changed
	if (!enableTask(false)) {
		return false;
	}
	// Assign loaded values
	current_config.pin = doc["pin"].as<int>();
	current_config.name = doc["name"].as<String>();
	current_config.onTime = doc["onTime"].as<String>();
	current_config.offTime = doc["offTime"].as<String>();
	current_config.enabled = doc["enabled"].as<bool>();
	current_config.active = doc["active"]["current"].as<std::string>();

	Description.name = current_config.name;
	TaskDescription.taskName = current_config.name.c_str();
	on_hour = current_config.onTime.substring(0, current_config.onTime.indexOf(':')).toInt();
	on_minute = current_config.onTime.substring(current_config.onTime.indexOf(':') + 1).toInt();
	off_hour = current_config.offTime.substring(0, current_config.offTime.indexOf(':')).toInt();
	off_minute = current_config.offTime.substring(current_config.offTime.indexOf(':') + 1).toInt();

	if (!saveConfig(config_path, getConfig()) || !enableTask(current_config.enabled)) {
		return false;
	}
	return configureOutput();
}

/// @brief Checks the time to see if the timer has triggered
/// @param elapsed The time in ms since this task was last called
void TimerSwitch::runTask(long elapsed) {
	totalElapsed += elapsed;
	if (current_config.enabled && totalElapsed >= TaskDescription.taskPeriod) {
		totalElapsed = 0;
		int cur_hour = rtc->getHour(true);
		int cur_min = rtc->getMinute();
		int cur_state = digitalRead(current_config.pin);
		if (cur_state != states[current_config.active] && cur_hour == on_hour) {
			if (cur_min == on_minute) {
				Serial.println("Timer switch turning on");
				digitalWrite(current_config.pin, states[current_config.active]);
			}
		} else if (cur_state == states[current_config.active] && cur_hour == off_hour) {
			if (cur_min == off_minute) {
				Serial.println("Timer switch turning off");
				digitalWrite(current_config.pin, !states[current_config.active]);
			}
		}
	}
}	