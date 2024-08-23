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

class Storage {
	public:
		bool begin();
		bool begin(int mi, int mo, int sck, int cs);
		bool begin(int clk, int cmd, int d0, int d1, int d2, int d3);

		/// @brief Enum of available storage media 
		enum class Media
		{
			SD_SPI,
			SD_MMC,
			LittleFS
		};
		
		static FS* getFileSystem();
		static Storage::Media getMediaType();
		std::vector<String> listDir(String dirname, uint8_t levels);
		bool fileExists(String path);
		bool createDir(String path);
		bool removeDir(String path);
		String readFile(String path);
		bool writeFile(String path, String content);
		bool appendToFile(String path, String content);
		bool renameFile(String path1, String path2);
		bool deleteFile(String path);
		size_t freeSpace();
		
	private:
		/// @brief The storage media type being used
		static Media storageMedia;

		/// @brief The file system being used
		static FS* storageSystem;	
};