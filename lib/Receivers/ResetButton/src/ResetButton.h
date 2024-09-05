/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <SignalReceiver.h>
#include <Storage.h>
#include <ArduinoJson.h>

/// @brief Provides a button to reset WiFi settings
class ResetButton : public SignalReceiver {
	private:
		Storage* storage;

		/// @brief Describes available pin states
		enum modes { NONE = 0x1, BUTTON_PULLUP = 0x5, BUTTON_PULLDOWN = 0x9 };

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
		
		bool saveConfig();
		void reset();
		void ResetChecker();
		static void ResetCheckerTaskWrapper(void* arg);

	public:
		/// @brief Holds the description of the device capable of receiving signals
		struct {
			/// @brief The number of signals this device can receive
			int signalQuantity;

			/// @brief The type of device this is
			String type;

			/// @brief The name of this device
			String name;
			
			/// @brief Contains of map of signals this device can receive and their ID numbers. Signal names must only alphanumeric and underscores, and contain at least on letter
			std::map<String, int> signals;

			/// @brief The ID of this device
			int id;
		} Description;

		ResetButton(Storage* Storage);
		bool begin();
		String receiveSignal(int signal, String payload = "");
		String getConfig();
		bool setConfig(String config);
};