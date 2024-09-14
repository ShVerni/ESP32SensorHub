#include "ResetButton.h"

/// @brief Starts a reset button
/// @return True on success
bool ResetButton::begin() {
	// Set description
	Description.signalQuantity = 1;
	Description.type = "button";
	Description.name = "Reset Button";
	Description.signals = {{"Reset", 0}};
	Description.id = 0;
	bool result = false;
	// Create settings directory if necessary
	if (!Storage::fileExists("/settings/sig/ResetButton.json")) {
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
		current_config = { .pin = D4, .mode = modes::BUTTON_PULLUP, .active = states::BUTTON_LOW };
		if (saveConfig()) {
			return configureButton();
		}
		return false;
	} else {
		// Load settings
		return setConfig(Storage::readFile("/settings/sig/ResetButton.json"));
	}
}

/// @brief Receives a signal
/// @param signal The signal to process (only option is 0 for reset)
/// @param payload Not used
/// @return JSON response with OK
std::tuple<bool, String> ResetButton::receiveSignal(int signal, String payload) {
	if (signal == 0) {
		reset();
	}	
	return { true, R"({"Response": "OK"})" };
}

/// @brief Gets the current config
/// @return A JSON string of the config
String ResetButton::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["pin"] = current_config.pin;
	doc["mode"] = current_config.mode;
	doc["active"] = current_config.active;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool ResetButton::setConfig(String config) {
	// Stop reset checker
	if(xCreated == pdPASS)
	{
		vTaskDelete(xHandle);
		xCreated = pdFAIL;
	}
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
	current_config.pin = doc["pin"].as<int>();
	current_config.active = doc["active"].as<states>();
	current_config.mode = doc["mode"].as<modes>();
	if (!saveConfig()) {
		return false;
	}
	return configureButton();
}

/// @brief Saves the current config to a JSON file
/// @return True on success
bool ResetButton::saveConfig() {
	return Storage::writeFile("/settings/sig/ResetButton.json", getConfig());
}

/// @brief Used to configure the button and start the reset checker task
/// @return True on success
bool ResetButton::configureButton() {
	pinMode(current_config.pin, current_config.mode);
	// Start the loop that checks for resets (could use an ISR instead but that has its own issues)
	if (xCreated != pdPASS) {
		xCreated = xTaskCreate(ResetCheckerTaskWrapper, "Reset Checker Loop", 8128, this, 1, &xHandle);
	}
	return xCreated == pdPASS;
}

/// @brief Wraps the reset checker task for static access
/// @param arg The ResetButton object
void ResetButton::ResetCheckerTaskWrapper(void* arg) {
	static_cast<ResetButton*>(arg)->ResetChecker();
}

/// @brief Checks if a reset was requested
void ResetButton::ResetChecker() {
	while (true) {
		if (shouldReset || digitalRead(current_config.pin) == current_config.active) {
			// Debounce
			delay(10);
			// Check if button is being held for 5 seconds
			int elapsed = 0;
			while(digitalRead(current_config.pin) == current_config.active) {
				delay(10);
				elapsed++;
				if (elapsed == 500) {
					reset();
					break;	
				}
			}
		}
		// This loop doesn't need to be tight
		delay(250);
	}
}

/// @brief Resets device settings
void ResetButton::reset() {
	Serial.println("Rest button pressed...");
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Rebooting);
	// Reset WiFi settings
	WiFi.mode(WIFI_AP_STA); // Cannot erase if not in STA mode!
	WiFi.persistent(true);
	WiFi.disconnect(true, true);
	WiFi.persistent(false);
	// Erase storage
	for (const auto& f : Storage::listFiles("/", 100)) {
		Storage::deleteFile(f);
	}
	for (const auto& d : Storage::listDirs("/", 100)) {
		Storage::removeDir(d);
	}
	ESP.restart();		
}