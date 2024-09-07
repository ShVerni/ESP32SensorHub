/*
 * This file and associated .cpp file are licensed under the GPLv3 License Copyright (c) 2024 Sam Groveman
 * 
 * Adapted from: https://docs.espressif.com/projects/arduino-esp32/en/latest/api/sdmmc.html
 * 
 * Contributors: Sam Groveman
 */

#pragma once
#include <FS.h>
#include <SD_MMC.h>
#include <LittleFS.h>
#include <SPI.h>
#include <SD.h>
#include <vector>

/// @brief Provides standardized access to various storage media
class Storage {
	public:
		/// @brief Enum of available storage media 
		enum class Media
		{
			SD_SPI,
			SD_MMC,
			LittleFS
		};
		
		static bool begin();
		static bool begin(int mi, int mo, int sck, int cs);
		static bool begin(int clk, int cmd, int d0, int d1, int d2, int d3);
		static FS* getFileSystem();
		static Storage::Media getMediaType();
		static std::vector<String> listDir(String dirname, uint8_t levels);
		static bool fileExists(String path);
		static bool createDir(String path);
		static bool removeDir(String path);
		static String readFile(String path);
		static bool writeFile(String path, String content);
		static bool appendToFile(String path, String content);
		static bool renameFile(String path1, String path2);
		static bool deleteFile(String path);
		static size_t freeSpace();
		
	private:
		/// @brief The storage media type being used
		static Media storageMedia;

		/// @brief The file system being used
		static FS* storageSystem;	
};