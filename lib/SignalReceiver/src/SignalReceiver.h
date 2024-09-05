/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <Arduino.h>
#include <map>

/// @brief Defines a generic signal receiver class for inheriting 
class SignalReceiver {
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

		virtual bool begin();
		virtual String receiveSignal(int signal, String payload = "");
		virtual String getConfig();
		virtual bool setConfig(String config);
};