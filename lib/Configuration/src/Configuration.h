/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <ArduinoJson.h>
#include <Storage.h>

/// @brief Holds and manages the hub configuration
class Configuration {

	public:
		struct {
			/// @brief Controls whether the sensor hub is enabled
			bool enabled = false;
			
			/// @brief Controls the sampling period of the sensor hub
			int period = 10000;

			/// @brief NTP server
			String ntpServer = "pool.ntp.org";

			/// @brief Daylight savings time offset in seconds
			int daylightOffset_sec = 3600;

			/// @brief Offset from GMT in seconds
			long  gmtOffset_sec = -18000;
		} currentConfig;

		Configuration(Storage* Storage, String File = "config.json");
		bool begin();
		bool loadConfig();
		bool updateConfig(String config);
		bool saveConfig();
		bool saveConfig(String config);
		String getConfig();

	private:
		/// @brief Pointer to the storage object to use
		Storage* storage;

		/// @brief Path to file to use to store/load settings
		String file;

		String configToJSON();

};