/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ArduinoJSON: https://arduinojson.org/
*
* Contributors: Sam Groveman
*/
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <SignalReceiver.h>

/// @brief Class describing a generic output on a GPIO pin
class GenericOutput : public SignalReceiver {
	protected:
		/// @brief Output configuration
		struct {
			/// @brief The pin number attached to the output
			int pin;

			/// @brief The name of this output
			String name;
		} current_config;

		/// @brief Path to configuration file
		String config_path;

		bool configureOutput();

	public:
		GenericOutput(int Pin, String configFile = "GenericOutput.json");
		bool begin();
		std::tuple<bool, String> receiveSignal(int signal, String payload = "");
		String getConfig();
		bool setConfig(String config);
};