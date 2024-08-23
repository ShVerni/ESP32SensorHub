/*
* This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
* 
* External libraries needed:
* FastLED: https://github.com/FastLED/FastLED
* 
* Contributors: Sam Groveman
*/

#pragma once
#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

class LEDIndicator {
		public:
		/// @brief Colors
		enum class Colors
		{
			Off,
			Red,
			Green,
			Blue,
			Yellow,
			Purple,
			Orange,
			Cyan,
			White
		};

		LEDIndicator(uint8_t LEDPin, int LEDCount, bool RGB = true);
		bool begin();
		void showColor(Colors color);
	
	private:
		/// @brief LED  driver
		Adafruit_NeoPixel leds;

		/// @brief Indicates if the LED is an RGB LED
		bool rgb;

		/// @brief The pin connected to the LED(s)
		int led_pin;

		/// @brief Array of color RGB hex codes
		int color_map[9] = {
		0x000000,     // Off    (0 blinks)
		0xFF0000,     // Red    (1 blinks)
		0x007F00,     // Green  (2 blinks)
		0x0000FF,     // Blue   (3 blinks)
		0xFF6000,     // Yellow (4 blinks)
		0xFF00C4,     // Purple (5 blinks)
		0xFF2800,     // Orange (6 blinks)
		0x00C4FF,     // Cyan	(7 blinks)
		0x909080      // White	(8 blinks)
	};
};