/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/
#pragma once
#include <Arduino.h>
#include <SignalReceiver.h>
#include <SensorManager.h>
#include <ArduinoJson.h>

/// @brief Allows retrieval of sensor data formatted for Prometheus ingest
class DataTemplate : public SignalReceiver {
	private:
		/// @brief Holds data logger configuration
		struct {
			/// @brief String to use for the start of the template
			String template_start;

			/// @brief String to use for the end of the template
			String template_end;

			/// @brief The template to use each line of the data
			String template_data;
		} current_config;

		/// @brief Full path to config file
		String config_path;

	public:
		DataTemplate(String ConfigFile = "DataTemplate.json");
		bool begin();
		std::tuple<bool, String> receiveSignal(int signal, String payload = "");
		String getConfig();
		bool setConfig(String config);;
};