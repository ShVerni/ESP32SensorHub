/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ArduinoJSON: https://arduinojson.org/
* 
* This could be done as a periodic task but is implemented with its own task loop so it can run even if there are errors starting
*
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <SignalReceiver.h>
#include <EventBroadcaster.h>
#include <ArduinoJson.h>
#include <unordered_map>

/// @brief Provides a button to factory reset device
class ResetButton : public SignalReceiver {
	private:
		/// @brief Describes available pin states
		std::unordered_map<std::string, int> modes = {{"Input", INPUT}, {"Input pull-up", INPUT_PULLUP}, {"Input pull-down", INPUT_PULLDOWN}};

		/// @brief Describes available button active states
		std::unordered_map<std::string, int> states = {{"Active low", LOW}, {"Active high", HIGH}};

		/// @brief Handle for task that checks for reset
		TaskHandle_t xHandle = NULL;

		/// @brief Used to check if the task was created
		BaseType_t xCreated = pdFAIL;

		
		/// @brief Button configuration
		struct {
			/// @brief The pin number attached to the button
			int pin;

			/// @brief The mode of the button
			std::string mode;
			
			/// @brief The active states of the button
			std::string active;
		} current_config;

		/// @brief When true will reset the WiFi and reboot
		bool shouldReset = false;

		/// @brief Path to configuration file
		const String config_path = "/settings/sig/ResetButton.json";

		/// @brief Event broadcaster object
		EventBroadcaster* event;

		bool configureButton();
		void reset();
		void ResetChecker();
		static void ResetCheckerTaskWrapper(void* arg);

	public:
		ResetButton(int Pin);
		bool begin();
		std::tuple<bool, String> receiveSignal(int signal, String payload = "");
		String getConfig();
		bool setConfig(String config);
};