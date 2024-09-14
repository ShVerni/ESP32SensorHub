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
#include <EventBroadcaster.h>
#include <SignalManager.h>
#include <WebServer.h>
#include <WiFiConfig.h>
#include <SensorManager.h>
#include <ResetButton.h>
#include <PeriodicTasks.h>
#include <LEDIndicator.h>
#include <LocalDataLogger.h>
#include <DataTemplate.h>

/// @brief Current firmware version
extern const String FW_VERSION = "0.5.0";

/// @brief Stores settings in NVS
Preferences settings;

/// @brief RTC object for getting/setting time
ESP32Time rtc;

/// @brief AsyncWebServer object (passed to WfiFiConfig and WebServer)
AsyncWebServer server(80);

/******** Declare sensor and receiver objects here ********/

/// @brief LED indicator object
LEDIndicator led(D8, 1);

/// @brief Reset button object
ResetButton reset_button;

/// @brief For logging data to local storage
LocalDataLogger logger(&rtc);

/// @brief For retrieving data formatted for Prometheus
DataTemplate schema_maker;

/******** End sensor and receiver object declaration ********/

void setup() {
	// Start serial
	Serial.begin(115200);
	Serial.print("Booting ESP32 sensor hub V");
	Serial.println(FW_VERSION);
	Serial.println();
	Serial.println("Designed and created by Sam Groveman (C) 2024");
	Serial.println();

	/******** Add event receivers and loggers here ********/

	EventBroadcaster::addReceiver(&led);

	/******** End event receivers and loggers addition section ********/

	if (!EventBroadcaster::beginReceivers())	{
		Serial.println("Could not start all event receivers");
		while(true);
	}

	// Show yellow during startup
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Starting);

	// Start preference storage on NVS
	if (!settings.begin("sensor-hub", false)) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Count not start NVS settings");
		while(true);
	};

	// Start storage
	if (!Storage::begin()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Could not start storage");
		while(true);
	}

	// Start configuration manager
	if (!Configuration::begin()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Could not start configuration manager");
		while(true);
	}

	if (Configuration::currentConfig.WiFiClient) {
		// Configure WiFi client
		DNSServer dns;
		AsyncWiFiManager manager(&server, &dns);
		WiFiConfig configurator(&manager, Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
		configurator.connectWiFi();
		WiFi.setAutoReconnect(true);
		server.reset();
		// Set local time via NTP
		configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer.c_str());
		Serial.println("Time set via NTP");
	} else {
		// Start AP
		WiFi.softAP(Configuration::currentConfig.configSSID, Configuration::currentConfig.configPW);
	}
	
	/// @brief Webserver handling all requests
	Webserver webserver(&server, &rtc);

	// Clear server settings, just in case
	webserver.ServerStop();

	// Start the update server
	webserver.ServerStart();
	xTaskCreate(Webserver::RebootCheckerTaskWrapper, "Reboot Checker Loop", 1024, &webserver, 1, NULL);

	// Load saved configuration if there is one
	if (!Configuration::loadConfig()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Could not load configuration");
		while(true);
	}

	/// Start webhooks
	if (!WebhookManager::begin("webhooks.json")) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Could not start webhook manager");
		while(true);
	}

	/******** Add sensors and receivers here ********/

	SignalManager::addReceiver(&reset_button);
	SignalManager::addReceiver(&logger);
	SignalManager::addReceiver(&schema_maker);

	/******** End sensor and receiver addition section ********/

	// Start sensors
	if (!SensorManager::beginSensors()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		while(true);
	}

	// Start receivers
	if (!SignalManager::beginReceivers()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		while(true);
	}

	// Load saved webhooks if any
	if (!WebhookManager::loadWebhooks()) {
		EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Error);
		Serial.println("Could not load webhooks");
		while(true);
	}

	// Print the configured sensors, receivers, and webhooks
	Serial.println(SensorManager::getSensorInfo());
	Serial.println(SignalManager::getReceiverInfo());
	Serial.println(WebhookManager::getWebhooks());

	// Start signal processor loop (8K of stack depth is probably overkill, but it does process potentially large JSON strings and we have the RAM, so better to be safe)
	xTaskCreate(SignalManager::signalProcessor, "Command Processor Loop", 8192, NULL, 1, NULL);

	// Ready!
	EventBroadcaster::broadcastEvent(EventBroadcaster::Events::Ready);
	Serial.println("System ready!");
}

// Used for tracking time intervals for timed events
ulong current_mills = 0;
ulong previous_mills_task = 0;

// Used to automatically synchronize clock at regular intervals
ulong previous_millis_ntp = 0;

void loop() {
	current_mills = millis();
	if(Configuration::currentConfig.WiFiClient) {
		// Synchronize the time every 6 hours
		if (current_mills - previous_millis_ntp > 21600000) {
			configTime(Configuration::currentConfig.gmtOffset_sec, Configuration::currentConfig.daylightOffset_sec, Configuration::currentConfig.ntpServer.c_str());
			previous_millis_ntp = current_mills;
		}
	}
	if (Configuration::currentConfig.tasksEnabled) {
		// Perform tasks periodically
		if (current_mills - previous_mills_task > Configuration::currentConfig.period) {
			PeriodicTasks::callTasks(current_mills - previous_mills_task);
			previous_mills_task = current_mills;
		}
	}
	delay(100);
}
