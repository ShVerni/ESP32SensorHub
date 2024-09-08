#include "Webserver.h"

/// @brief Used to tell if operating in WiFi client mode
extern const bool WiFiClient;

/// @brief Holds current firmware version
extern const String FW_VERSION;

// Initialize static variables
bool Webserver::upload_abort = false;
int Webserver::upload_response_code = 201;

/// @brief Creates a Webserver object
/// @param Webserver A pointer to an AsyncWebServer object
/// @param RTC A pointer to a ESP32Time object
Webserver::Webserver(AsyncWebServer* Webserver, ESP32Time* RTC) {
	server = Webserver;
	rtc = RTC;
}

/// @brief Starts the update server
bool Webserver::ServerStart() {
	Serial.println("Starting web server");

	// Create root directory if needed
	if (!Storage::fileExists("/www"))
		if (!Storage::createDir("/www"))
			return false;

	// Add request handler for index page
	if (Storage::fileExists("/www/index.html")) {
		// Serve any page from filesystem
		server->serveStatic("/", *Storage::getFileSystem(), "/www/").setDefaultFile("index.html");
	} else {
		// Serve the embedded index page
		server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
			request->send_P(HTTP_CODE_OK, "text/html", index_page);
		});
	}

	// Handle file uploads
	server->on("/upload-file", HTTP_POST, [](AsyncWebServerRequest *request) {
		// Let upload start
		delay(50);
		// Construct response
		AsyncWebServerResponse *response = request->beginResponse(Webserver::upload_response_code, "text/plain", Webserver::upload_abort ? "Upload failed": "File uploaded");
		response->addHeader("Connection", "close");
		request->send(response);
	}, onUpload_file);

	// Handle deletion of files
	server->on("/delete", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if(request->hasParam("path", true)) {
			String path = request->getParam("path", true)->value();
			Serial.println("Deleting " + path);
			if (Storage::fileExists(path)) {
				bool success = Storage::deleteFile(path);
				request->send(HTTP_CODE_OK, "text/plain", success ? "OK" : "FAIL");
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Get descriptions of available sensors
	server->on("/sensors", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", SensorManager::getSensorInfo());
	});

	// Get curent configuration of a sensor
	server->on("/sensors/sensor", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("sensor")) {
			int sensorPosID = request->getParam("sensor")->value().toInt();
			request->send(HTTP_CODE_OK, "text/json", SensorManager::getSensorConfig(sensorPosID));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Update configuration of a sensor
	server->on("/sensors/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("sensor", true)) {
			// Parse data payload
			int sensorPosID = request->getParam("sensor", true)->value().toInt();
			String config = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (SensorManager::setSensorConfig(sensorPosID, config)) {
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Gets last measurement. Add GET paramater "update" (/sensors/measurement?update) to take a new measurement first
	server->on("/sensors/measurement", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("update")) {
			// Attempt to take new measurement
			if (!SensorManager::takeMeasurement()) {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not take measurement");
				return;
			}
		}
		request->send(HTTP_CODE_OK, "text/json", SensorManager::getLastMeasurement());
	});
	
	// Runs a calibration procedure on a sensor
	server->on("/sensors/calibrate", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("sensor", true) && request->hasParam("step", true)) {
			// Parse data payload
			int sensorPosID = request->getParam("sensor", true)->value().toInt();
			int step = request->getParam("step", true)->value().toInt();

			// Run sensor calibration
			std::tuple<Sensor::calibration_response, String> response = SensorManager::calibrateSensor(sensorPosID, step);

			// Create response
			request->send(HTTP_CODE_OK, "text/json", "{ \"response\":" + String(std::get<0>(response)) + ",\"message\":" + std::get<1>(response) + "}");
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Get descriptions of available signal receivers
	server->on("/signals", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", SignalManager::getReceiverInfo());
	});

	// Get curent configuration of a receiver
	server->on("/signals/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("receiver")) {
			int receiverPosID = request->getParam("sensor")->value().toInt();
			request->send(HTTP_CODE_OK, "text/json", SignalManager::getReceiverConfig(receiverPosID));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Update configuration of a receiver
	server->on("/signals/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("receiver", true)) {
			// Parse data payload
			int receiverPosID = request->getParam("receiver", true)->value().toInt();
			String config = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (SignalManager::setReceiverConfig(receiverPosID, config)) {
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Adds a signal to the signal queue using the signal's name
	server->on("/signals/name", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("receiver", true) && request->hasParam("signal", true)) {
			// Parse data payload
			int receiverPosID = request->getParam("receiver", true)->value().toInt();
			String signalName = request->getParam("signal", true)->value();
			String payload = "";
			if (request->hasParam("payload", true)) {
				payload = request->getParam("payload", true)->value();
			}
			// Attempt to add signal to queue
			if (!SignalManager::addSignalToQueue(receiverPosID, signalName, payload)) {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not add signal to queue");
			}
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Adds a signal to the signal queue using the signal's ID number
	server->on("/signals/id", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("receiver", true) && request->hasParam("signal", true)) {
			int receiverPosID = request->getParam("receiver", true)->value().toInt();
			int signalID = request->getParam("signal", true)->value().toInt();
			String payload = "";
			if (request->hasParam("payload", true)) {
				payload = request->getParam("payload", true)->value();
			}
			// Attempt to add signal to queue
			if (!SignalManager::addSignalToQueue(receiverPosID, signalID, payload)) {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not add signal to queue");
			}
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Sends a signal to a receiver immediately using the signal's name, and returns response
	server->on("/signals/immediateSignal", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("receiver", true) && request->hasParam("signal", true)) {
			int receiverPosID = request->getParam("receiver", true)->value().toInt();
			String signalName = request->getParam("signal", true)->value();
			String payload = "";
			if (request->hasParam("payload", true)) {
				payload = request->getParam("payload", true)->value();
			}
			// Execute signal and return response
			request->send(HTTP_CODE_OK, "text/json", SignalManager::processSignalImmediately(receiverPosID, signalName, payload));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Sends a signal to a receiver immediately using the signal's ID number, and returns response
	server->on("/signals/immediateSignalID", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("receiver", true) && request->hasParam("signal", true)) {
			int receiverPosID = request->getParam("receiver", true)->value().toInt();
			int signalID = request->getParam("signal", true)->value().toInt();
			String payload = "";
			if (request->hasParam("payload", true)) {
				payload = request->getParam("payload", true)->value();
			}
			// Execute signal and return response
			request->send(HTTP_CODE_OK, "text/json", SignalManager::processSignalImmediately(receiverPosID, signalID, payload));
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Get curent global configuration
	server->on("/config", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", Configuration::getConfig());
	});

	// Update global configuration
	server->on("/config", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("config", true) && request->hasParam("save", true)) {
			// Parse data payload
			bool save = request->getParam("save", true)->value() == "1";
			String config_string = request->getParam("config", true)->value();
			// Attempt to apply config data
			if (Configuration::updateConfig(config_string)) {
				if (save) {
					// Attempt to save config
					if(!Configuration::saveConfig(config_string)) {
						request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not save config settings");
						return;
					}
				}
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply config settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Get curent webhooks
	server->on("/webhooks", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", WebhookManager::getWebhooks());
	});

	// Update webhooks
	server->on("/webhooks", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("webhooks", true) && request->hasParam("save", true)) {
			// Parse data payload
			bool save = request->getParam("save", true)->value() == "1";
			String webhooks_string = request->getParam("webhooks", true)->value();
			// Attempt to apply config data
			if (WebhookManager::updateWebhooks(webhooks_string)) {
				if (save) {
					// Attempt to save config
					if(!WebhookManager::saveWebhooks(webhooks_string)) {
						request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not save webhook settings");
						return;
					}
				}
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", "Could not apply webhook settings");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Fires a webhook using a GET request and the webhook's position ID
	server->on("/webhook/get", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("webhook", true) && request->hasParam("type", true)) {
			int webhookPosID = request->getParam("receiver", true)->value().toInt();
			String type = request->getParam("type", true)->value();
			type.toLowerCase();
			if (request->hasParam("parameters", true)) {
				String parameters = request->getParam("parameters", true)->value();
				if (type == "json") {
					request->send(HTTP_CODE_OK, "text/json", WebhookManager::fireGet(webhookPosID, parameters));
				} else {
					// Convert from JSON to std::map<String, String>
					// Allocate the JSON document
					JsonDocument doc;
					// Deserialize file contents
					DeserializationError error = deserializeJson(doc, parameters);
					// Test if parsing succeeds.
					if (error) {
						Serial.print(F("Deserialization failed: "));
						Serial.println(error.f_str());
						request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad JSON parameter data");
						return;
					}
					// Build parameter map
					std::map<String, String> params;
					for (JsonPair param : doc.as<JsonObject>()) {
						params[param.key().c_str()] = param.value().as<String>();
					}
					request->send(HTTP_CODE_OK, "text/json", WebhookManager::fireGet(webhookPosID, params));
				}

			} else {
				request->send(HTTP_CODE_OK, "text/json", WebhookManager::fireGet(webhookPosID));
			}
			
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Fires a webhook using a POST request and the webhook's position ID
	server->on("/webhook/post", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("webhook", true) && request->hasParam("type", true) && request->hasParam("parameters", true)) {
			int webhookPosID = request->getParam("receiver", true)->value().toInt();
			String type = request->getParam("type", true)->value();
			type.toLowerCase();
			String parameters = request->getParam("parameters", true)->value();
			if (type == "json") {
				request->send(HTTP_CODE_OK, "text/json", WebhookManager::fireGet(webhookPosID, parameters));
			} else {
				// Convert from JSON to std::map<String, String>
				// Allocate the JSON document
				JsonDocument doc;
				// Deserialize file contents
				DeserializationError error = deserializeJson(doc, parameters);
				// Test if parsing succeeds.
				if (error) {
					Serial.print(F("Deserialization failed: "));
					Serial.println(error.f_str());
					request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad JSON parameter data");
					return;
				}
				// Build parameter map
				std::map<String, String> params;
				for (JsonPair param : doc.as<JsonObject>()) {
					params[param.key().c_str()] = param.value().as<String>();
				}
				request->send(HTTP_CODE_OK, "text/json", WebhookManager::firePost(webhookPosID, params));
			}			
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Sets the time on the device (example of parsing JSON parameters)
	server->on("/setTime", HTTP_POST, [this](AsyncWebServerRequest *request) {
		if (!WiFiClient) {
			if (request->hasParam("time", true)) {
				// Parse data payload
				String time_data_string = request->getParam("time", true)->value();
				JsonDocument time_data;
				DeserializationError error = deserializeJson(time_data, time_data_string);
				// Test if parsing succeeds.
				if (error) {
					Serial.print(F("Deserialization failed: "));
					Serial.println(error.f_str());
					request->send(HTTP_CODE_BAD_REQUEST, "text/plain", error.f_str());
					return;
				}
				long time = time_data["time"];
				long offset = time_data["offset"];
				
				// Apply time settings
    			rtc->setTime(time);
				rtc->offset = offset;

				Serial.print("Set time and timezone offset to: ");
				Serial.print(time);
				Serial.print(" ");
				Serial.println(offset);
				request->send(HTTP_CODE_OK, "text/plain", "OK");
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
			}
		} else {
			request->send(HTTP_CODE_OK, "text/plain", "OK");
			Serial.println("Time already set by NTP");
		}
	});

	// Handle request for the amount of free space on the storage device (example of returning JSON data)
	server->on("/freeSpace", HTTP_GET, [this](AsyncWebServerRequest *request) {	
		String result = "{ \"space\": " + String(Storage::freeSpace()) + " }";
		request->send(HTTP_CODE_OK, "text/json", result);
	});

	// Handle reset request
	server->on("/reset", HTTP_PUT, [this](AsyncWebServerRequest *request) {
		Serial.println("Resetting WiFi settings");
		 if (Storage::fileExists("/www/reset.html")) {
			request->send(*Storage::getFileSystem(), "/www/reset.html", "text/html");
		} else {
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		}
		WiFi.mode(WIFI_AP_STA); // cCannot erase if not in STA mode !
		WiFi.persistent(true);
		WiFi.disconnect(true, true);
		WiFi.persistent(false);
		shouldReboot = true;
	});

	// Handle reboot request
	server->on("/reboot", HTTP_PUT, [this](AsyncWebServerRequest *request) {
		if (Storage::fileExists("/www/reboot.html")) {
			request->send(*Storage::getFileSystem(), "/www/reboot.html", "text/html");
		} else {
			request->send(HTTP_CODE_OK, "text/plain", "OK");
		}
		this->shouldReboot = true;
	});

	// Handle listing files
	server->on("/list", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("path")) {
			String path = request->getParam("path")->value();
			if (Storage::fileExists(path)) {
				int depth = 0;
				if (request->hasParam("depth")) {
					depth = request->getParam("depth")->value().toInt();
				}
				std::vector<String> file_list = Storage::listDir(path, depth);
				JsonDocument files;
				for (int i = 0; i < file_list.size(); i++) {
					files["files"][i] = file_list[i];
				}
				String files_string;
				serializeJson(files, files_string);
				request->send(HTTP_CODE_OK, "text/json", files_string);
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Folder doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Handle downloads
	server->on("/download", HTTP_GET, [this](AsyncWebServerRequest *request) {
		if (request->hasParam("path")) {
			String path = request->getParam("path")->value();
			if (Storage::fileExists(path)) {
				request->send(*Storage::getFileSystem(), path, "application/octet-stream");
			} else {
				request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "File doesn't exist");
			}
		} else {
			request->send(HTTP_CODE_BAD_REQUEST, "text/plain", "Bad request data");
		}
	});

	// Update page is special and hard-coded to always be available
	server->on("/update", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send_P(HTTP_CODE_OK, "text/html", update_page);
	});

	// Used to fetch current firmware version
	server->on("/version", HTTP_GET, [this](AsyncWebServerRequest *request) {
		request->send(HTTP_CODE_OK, "text/json", "{\"version\":\"" + FW_VERSION + "\"}");
	});

	// Update firmware
	server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
		// Let update start
		delay(50);
		
		// Check if should reboot
		shouldReboot = !Update.hasError();

		// Construct response
		AsyncWebServerResponse *response = request->beginResponse(this->shouldReboot ? HTTP_CODE_ACCEPTED : HTTP_CODE_INTERNAL_SERVER_ERROR, "text/plain", this->shouldReboot ? "OK" : "ERROR");
		response->addHeader("Connection", "close");
		request->send(response);
	}, onUpdate);    

	// 404 handler
	server->onNotFound([](AsyncWebServerRequest *request) { 
		request->send(HTTP_CODE_NOT_FOUND); 
	});

	server->begin();
	return true;
}

/// @brief Stops the update server
void Webserver::ServerStop() {
	Serial.println("Stopping web server");
	server->reset();
	server->end();
}

/// @brief Wraps the reboot checker task for static access
/// @param arg The Webserver object
void Webserver::RebootCheckerTaskWrapper(void* arg) {
	static_cast<Webserver*>(arg)->RebootChecker();
}

/// @brief Checks if a reboot was requested
void Webserver::RebootChecker() {
	while (true) {
		if (shouldReboot) {
			Serial.println("Rebooting...");
			// Delay to show LED and let server send response
			EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Rebooting);
			delay(3000);
			ESP.restart();
		}
		// This loop doesn't need to be tight
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

/// @brief Handle file uploads to a folder. Adapted from https://github.com/smford/esp32-asyncwebserver-fileupload-example
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpload_file(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	if (!index) {
		if (!request->hasHeader("FILE_UPLOAD_PATH")) {
			final = true;
			Webserver::upload_abort = true;
			Webserver::upload_response_code = HTTP_CODE_BAD_REQUEST;
			return;
		}
		String path = request->header("FILE_UPLOAD_PATH");
		Webserver::upload_abort = false;
		request->_tempFile = Storage::getFileSystem()->open(path + "/" + filename, "w", true);
		Serial.println("Uploading file " + filename);
	}
	if (Webserver::upload_abort)
		return;
	if (len) {
		// Stream the incoming chunk to the opened file
		if (request->_tempFile.write(data, len) != len) {
			final = true;
			Webserver::upload_abort = true;
			Webserver::upload_response_code = HTTP_CODE_INSUFFICIENT_STORAGE;
		}
	}
	if (final) {
		// Close the file handle as the upload is now done
		request->_tempFile.close();
		if (Webserver::upload_abort) {
			// Remove failed upload
			Storage::getFileSystem()->remove(request->_tempFile.path());
		} else {
			Webserver::upload_response_code = HTTP_CODE_CREATED;
		}
	}
}

/// @brief Handle firmware update
/// @param request
/// @param filename
/// @param index
/// @param data
/// @param len
/// @param final
void Webserver::onUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
	if (!index)
	{
		Serial.printf("Update Start: %s\n", filename.c_str());
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Updating);
		// Ensure firmware will fit into flash space
		if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000))
		{
			Update.printError(Serial);
		}
	}
	if (!Update.hasError())
	{
		if (Update.write(data, len) != len)
		{
			Update.printError(Serial);
		}
	}
	if (final)
	{
		if (Update.end(true))
		{
			Serial.printf("Update Success: %uB\n", index + len);
		}
		else
		{
			Update.printError(Serial);
		}
	}
}