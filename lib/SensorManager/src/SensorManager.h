/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* ArduinoJSON: https://arduinojson.org/
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Sensor.h>
#include <vector>
#include <ArduinoJson.h>

class SensorManager {
	private:
		/// @brief Collects all the senors that are in use
		std::vector<Sensor*> sensors;

		/// @brief Describes all info associated with a measurement
		struct measurement {
			/// @brief The parameter being measured
			String parameter;

			/// @brief The value of the measurement
			double value;

			/// @brief The unit of the measurement
			String unit;
		};

	public:
		/// @brief Contains the most recently requested measurements
		std::vector<measurement> measurements;

		bool addSensor(Sensor* sensor);
		bool beginSensors();
		bool takeMeasurement();
		String getLastMeasurement();
		String getSensorInfo();
		String getSensorConfig(int sensorPosID);
		bool setSensorConfig(int sensorPosID, String config);
		std::tuple<Sensor::calibration_response, String> calibrateSensor(int sensorPosID, int step);
};