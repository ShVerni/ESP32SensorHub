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
		/// @brief Describes a signal receiver
		typedef struct receiver_info {
			/// @brief Ths positional ID of the receiver (where it's located in the vector)
			int positionID;

			/// @brief A pointer to the receiver object
			SignalReceiver* receiver;
		} receiver_info;

		/// @brief Stores all the in-use signal receivers
		std::vector<receiver_info> receivers;

		/// @brief Queue to hold signals to be processed.
        QueueHandle_t signalQueue;

		/// @brief Holds all payloads delivered with a signal
		std::queue<String> payloads;

		void processSignal();

	public:
		SignalManager();
		bool addReceiver(SignalReceiver* receiver);
		bool addSignalToQueue(int receiverPosID, String signal, String payload = "");
		bool addSignalToQueue(int receiverPosID, int signal, String payload = "");
		String processSignalImmediately(int receiverPosID, String signal, String payload = "");
		String processSignalImmediately(int receiverPosID, int signal, String payload = "");
		String getReceiverInfo();
		static void SignalProcessorTaskWrapper(void* signalManager);
};