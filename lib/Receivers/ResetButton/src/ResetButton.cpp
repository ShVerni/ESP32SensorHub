#include "ResetButton.h"

/// @brief Creates a reset button object
/// @param Storage A storage object to use to store settings
ResetButton::ResetButton(Storage* Storage) {
	storage = Storage;
}

/// @brief Starts a reset button
/// @return True on success
bool ResetButton::begin() {
	// Set description
	Description.signalQuantity = 1;
	Description.type = "button";
	Description.name = "Reset Button";
	Description.signals = {{"Reset", 0}};
	Description.id = 0;
	// Create settings directory if necessary
	if (!storage->fileExists("/settings/sig")) {
		if (!storage->fileExists("/settings")) {
			if (!storage->createDir("/settings")) {
				return false;
			}
		}
		if (!storage->createDir("/settings/sig")) {
				return false;
		}
	}
	// Load settings
	bool result = false;
	if (!storage->fileExists("/settings/sig/ResetButton.json")) {
		// Set defaults
		current_config = { .pin = D4, .mode = modes::BUTTON_PULLUP, .active = states::BUTTON_LOW };
		result = saveConfig();
	} else {
		result = setConfig(storage->readFile("/settings/sig/ResetButton.json"));
	}
	// Config button pin
	if (result) {
		pinMode(current_config.pin, current_config.mode);
	}
	// Start the loop that checks for resets (could use an ISR but that has its own issues)
	xTaskCreate(ResetCheckerTaskWrapper, "Reset Checker Loop", 1024, this, 1, NULL);
	return result;
}

/// @brief Receives a signal
/// @param signal The signal to process (only option is 0 for reset)
/// @param payload Not used
/// @return 
String ResetButton::receiveSignal(int signal, String payload) {
	if (signal == 0) {
		reset();
	}	
	return R"({"Response": "OK"})";
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

bool ResetButton::setConfig(String config) {
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
	return true;
}

/// @brief Saves the current config to a JSON file
/// @return True on success
bool ResetButton::saveConfig() {
	return storage->writeFile("/settings/sig/ResetButton.json", getConfig());
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
			reset();
		}
		// This loop doesn't need to be tight
		delay(50);
	}
}

/// @brief Resets WiFi settings
void ResetButton::reset() {
	WiFi.mode(WIFI_AP_STA); // Cannot erase if not in STA mode!
	WiFi.persistent(true);
	WiFi.disconnect(true, true);
	WiFi.persistent(false);
	Serial.println("Rebooting...");
	delay(3000);
	ESP.restart();		
}