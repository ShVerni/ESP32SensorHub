/*
* This project and associated original files are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman 
* Contributors: Sam Groveman
*/

#include <Arduino.h>
#include <Wire.h>
#include <Preferences.h>
#include <Webserver.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncWiFiManager.h>
#include <ESP32Time.h>
#include <Storage.h>
#include <WebhookManager.h>
#include <Configuration.h>
#include <LEDIndicator.h>
#include <SignalManager.h>
#include <WebServer.h>
#include <SensorManager.h>
#include <ResetButton.h>

/// @brief Current firmware version
extern const String FW_VERSION = "0.5.0";

// Uncomment the below to enable online WiFi client mode, comment out to operate offline as an AP
#define WIFI_CLIENT

#ifdef WIFI_CLIENT
	extern const bool WiFiClient = true;
	#include <WiFiConfig.h>
	// Button to clear saved WiFi client settings
#else
	extern const bool WiFiClient = false;
#endif

/// @brief Stores settings in NVS
Preferences settings;

/// @brief Storage object
Storage storage;

/// @brief Configuration object for storing full config
Configuration config(&storage);

/// @brief RTC object for getting/setting time
ESP32Time rtc;

/// @brief AsyncWebServer object (passed to WfiFiConfig and WebServer)
AsyncWebServer server(80);

/// @brief LED indicator object
LEDIndicator led(D8, 1);

/// @brief Sensor manager object
SensorManager sensors;

/// @brief Signal manager object
SignalManager receivers;

WebhookManager webhooks(&storage, "webhooks.json");

/******** Declare sensor and receiver objects here ********/

ResetButton reset_button(&storage);

/******** End sensor and receiver object declaration ********/

void setup() {
	// Start serial
	Serial.begin(115200);
	Serial.print("Booting ESP32 sensor hub V");
	Serial.println(FW_VERSION);
	Serial.println();
	Serial.println("Designed and created by Sam Groveman (C) 2024");
	Serial.println();

	// Start LEDs
	led.begin();

	// Show yellow during startup
	led.showColor(LEDIndicator::Colors::Yellow);

	// Start preference storage on NVS
	if (!settings.begin("sensor-hub", false)) {
		led.showColor(LEDIndicator::Colors::Red);
		Serial.println("Count not start NVS settings");
		while(true);
	};

	// Start storage
	if (!storage.begin()) {
		led.showColor(LEDIndicator::Colors::Red);
		Serial.println("Could not start storage");
		while(true);
	}

	// Start configuration manager
	if (!config.begin()) {
		led.showColor(LEDIndicator::Colors::Red);
		Serial.println("Could not start configuration manager");
		while(true);
	}

	// TEMPORARY: Create dummy config here until web interface is built
	config.currentConfig.enabled = true;
	config.saveConfig();

	// Load saved configuration if there is one
	config.loadConfig();

	#ifdef WIFI_CLIENT
		// Configure WiFi client
		DNSServer dns;
		AsyncWiFiManager manager(&server, &dns);
		WiFiConfig configurator(&manager, &led, config.currentConfig.configSSID, config.currentConfig.configPW);
		configurator.connectWiFi();
		WiFi.setAutoReconnect(true);
		server.reset();
		// Set local time via NTP
		configTime(config.currentConfig.gmtOffset_sec, config.currentConfig.daylightOffset_sec, config.currentConfig.ntpServer.c_str());
		Serial.println("Time set via NTP");
	#else
		// Start AP
		WiFi.softAP(config.currentConfig.configSSID, config.currentConfig.configPW);
	#endif
	
	/// @brief Webserver handling all requests
	Webserver webserver(&server, &storage, &led, &rtc, &sensors, &receivers, &config, &webhooks);

	// Clear server settings, just in case
	webserver.ServerStop();

	// Start the update server
	webserver.ServerStart();
	xTaskCreate(Webserver::RebootCheckerTaskWrapper, "Reboot Checker Loop", 1024, &webserver, 1, NULL);

	/******** Add sensors and receivers here ********/

	receivers.addReceiver(&reset_button);

	/******** End sensor and receiver addition section ********/

	// Start sensors
	if (!sensors.beginSensors()) {
		led.showColor(LEDIndicator::Colors::Red);
		while(true);
	}

	// Start receivers
	if (!receivers.beginReceivers()) {
		led.showColor(LEDIndicator::Colors::Red);
		while(true);
	}

	// Print the configured sensors and receivers
	Serial.println(sensors.getSensorInfo());
	Serial.println(receivers.getReceiverInfo());

	// Load webhooks, if any
	webhooks.loadWebhooks();

	// Start signal processor loop (8K of stack depth is probably overkill, but it does process potentially large JSON strings and we have the RAM, so better to be safe)
	xTaskCreate(SignalManager::SignalProcessorTaskWrapper, "Command Processor Loop", 8192, &receivers, 1, NULL);

	// Ready!
	led.showColor(LEDIndicator::Colors::Cyan);
	Serial.println("System ready!");
}

// Used for tracking time intervals for timed events
ulong current_mills = 0;
ulong previous_mills_measure = 0;

#ifdef WIFI_CLIENT
	// Used to automatically synchronize clock at regular intervals
	ulong previous_millis_ntp = 0;
#endif

void loop() {
	current_mills = millis();
	#ifdef WIFI_CLIENT
		// Synchronize the time every 6 hours
		if (current_mills - previous_millis_ntp > 21600000) {
			configTime(config.currentConfig.gmtOffset_sec, config.currentConfig.daylightOffset_sec, config.currentConfig.ntpServer.c_str());
			previous_millis_ntp = current_mills;
		}
	#endif
	if (config.currentConfig.enabled) {
		// Perform actions periodically
		if (current_mills - previous_mills_measure > config.currentConfig.period) {
			if (sensors.takeMeasurement()) {
				// Do something with measurements
				for (const auto &m : sensors.measurements) {
					Serial.println(m.parameter + ": " + m.value + " " + m.unit);
				}
			} else {
				Serial.println("Measurement failed");
			}
			previous_mills_measure = current_mills;
		}
	}
	delay(50);
}
