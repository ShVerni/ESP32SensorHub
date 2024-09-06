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
#include <SignalReceiver.h>
#include <vector>
#include <queue>


/// @brief Receives and processes signals for signal receivers
class SignalManager {
	private:
		/// @brief Stores all the in-use signal receivers
		std::vector<SignalReceiver*> receivers;

		/// @brief Queue to hold signals to be processed.
        QueueHandle_t signalQueue;

		/// @brief Holds all payloads delivered with a signal
		std::queue<String> payloads;

		void processSignal();

	public:
		SignalManager();
		bool addReceiver(SignalReceiver* receiver);
		bool beginReceivers();
		bool addSignalToQueue(int receiverPosID, String signal, String payload = "");
		bool addSignalToQueue(int receiverPosID, int signal, String payload = "");
		String processSignalImmediately(int receiverPosID, String signal, String payload = "");
		String processSignalImmediately(int receiverPosID, int signal, String payload = "");
		String getReceiverInfo();
		String getReceiverConfig(int receiverPosID);
		bool setReceiverConfig(int receiverPosID, String config);
		static void SignalProcessorTaskWrapper(void* signalManager);
};