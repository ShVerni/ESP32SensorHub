/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ArduinoJSON: https://arduinojson.org/
* 
* This could be done as a periodic task but is implemented with its own task loop as an example for how to do that
*
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <SignalReceiver.h>
#include <Storage.h>
#include <EventBroadcaster.h>
#include <ArduinoJson.h>

/// @brief Provides a button to reset WiFi settings
class ResetButton : public SignalReceiver {
	private:
		/// @brief Describes available pin states
		enum modes { NONE = 0x1, BUTTON_PULLUP = 0x5, BUTTON_PULLDOWN = 0x9 };

		/// @brief Handle for task that checks for reset
		TaskHandle_t xHandle = NULL;

		/// @brief Used to check if the task was created
		BaseType_t xCreated = pdFAIL;

		/// @brief Describes available button active states
		enum states { BUTTON_LOW = 0x0, BUTTON_HIGH = 0x1 };
		
		/// @brief Button configuration
		struct {
			/// @brief The pin number attached to the button
			int pin;

			/// @brief The mode of the button
			modes mode;
			
			/// @brief The active states of the button
			states active;
		} current_config;

		/// @brief When true will reset the WiFi and reboot
		bool shouldReset = false;

		/// @brief Event broadcaster object
		EventBroadcaster* event;
		
		bool saveConfig();
		bool configureButton();
		void reset();
		void ResetChecker();
		static void ResetCheckerTaskWrapper(void* arg);

	public:
		bool begin();
		std::tuple<bool, String> receiveSignal(int signal, String payload = "");
		String getConfig();
		bool setConfig(String config);
};