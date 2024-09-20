#include"GenericOutput.h"

/// @brief Creates a generic output
/// @param Pin Pin to use
/// @param configFile Name of the config file to use
GenericOutput::GenericOutput(int Pin, String configFile) {
	config_path = "/settings/sig/" + configFile;
	current_config.pin = Pin;
}

/// @brief Starts an output 
/// @return True on success
bool GenericOutput::begin() {
	// Set description
	Description.signalQuantity = 1;
	Description.type = "output";
	Description.name = "Generic Output";
	Description.signals = {{"state", 0}};
	Description.id = 0;
	bool result = false;
	// Create settings directory if necessary
	if (!checkConfig(config_path)) {
		// Set defaults
		return setConfig(R"({ "pin":)" + String(current_config.pin) + R"(, "name": "Generic Output" })");
	} else {
		// Load settings
		return setConfig(Storage::readFile(config_path));
	}
}

/// @brief Receives a signal
/// @param signal The signal to process (only option is 0 for reset)
/// @param payload A 0 or 1 to set the pin high or low
/// @return JSON response with OK
std::tuple<bool, String> GenericOutput::receiveSignal(int signal, String payload) {
	if (signal == 0) {
		digitalWrite(current_config.pin, payload.toInt());
	}	
	return { true, R"({"Response": "OK"})" };
}

/// @brief Gets the current config
/// @return A JSON string of the config
String GenericOutput::getConfig() {
	// Allocate the JSON document
	JsonDocument doc;
	// Assign current values
	doc["pin"] = current_config.pin;
	doc["name"] = current_config.name;

	// Create string to hold output
	String output;
	// Serialize to string
	serializeJson(doc, output);
	return output;
}

/// @brief Sets the configuration for this device
/// @param config The JSON config to use
/// @return True on success
bool GenericOutput::setConfig(String config) {
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
	current_config.name = doc["name"].as<String>();
	Description.name = current_config.name;

	if (!saveConfig(config_path, getConfig())) {
		return false;
	}
	return configureOutput();
}

/// @brief Configures the pin for use
/// @return True on success
bool GenericOutput::configureOutput() {
	pinMode(current_config.pin, OUTPUT);
	return true;
}