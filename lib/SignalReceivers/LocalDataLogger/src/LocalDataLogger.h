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
#include <PeriodicTasks.h>
#include <EventBroadcaster.h>
#include <SensorManager.h>
#include <ESP32Time.h>
#include <Storage.h>
#include <ArduinoJson.h>

/// @brief Logs sensor data locally
class LocalDataLogger : public SignalReceiver {
	private:
		/// @brief Holds data logger configuration
		struct {
			/// @brief The file name and used to log data in data directory
			String name;

			/// @brief Enable data logging
			bool enabled;
		} current_config;

		/// @brief CSV column header
		String header;

		/// @brief Full path to data file
		String path;

		/// @brief Pointer to the clock object in use
		ESP32Time* rtc;

		bool saveConfig();
		bool enableLogging(bool enable);

	public:
		LocalDataLogger(ESP32Time* RTC);
		bool begin();
		String getConfig();
		bool setConfig(String config);
		void logData();
};