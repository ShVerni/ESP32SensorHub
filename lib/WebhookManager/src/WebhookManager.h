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
#include <Webhook.h>
#include <Storage.h>
#include <vector>
#include <map>

class WebhookManager {
	private:
	typedef struct Webhook_info {
			/// @brief Ths positional ID of the webhook (where it's located in the vector)
			int positionID;

			/// @brief A pointer to the receiver object
			std::shared_ptr<Webhook> hook;
		} Webhook_info;

		/// @brief Stores all the in-use webhooks
		std::vector<Webhook_info> webhooks;

		/// @brief Full path to config file
		String config;

		/// @brief Pointer to the storage object to use
		Storage* storage;
		
		String hooksToJSON();

	public:
		WebhookManager(Storage* Storage, String configFile);
		bool loadWebhooks();
		bool saveWebhooks();
		bool saveWebhooks(String hooks);
		bool updateWebhooks(String hooks);
		String getWebhooks();
		String fireGet(int positionID);
		String fireGet(int positionID, String parameters);
		String fireGet(int positionID, std::map<String,String> parameters);
		String firePost(int positionID, String parameters);
		String firePost(int positionID, std::map<String,String> parameters);
};