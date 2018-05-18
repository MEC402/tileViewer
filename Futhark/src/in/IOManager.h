#pragma once
#include <vector>
#include <string>
#include "../base/Utility.h"
namespace fk {


	/* Used to load input files. */
	class IOManager {
	public:
		/* Reads a binary file into a vector of chars.
		(filePath) The path to the file to load.
		(buffer)< The vector to fill.
		< If the file read was successful */
		bool readBinaryFileToBuffer(
			const std::string& filePath,
			std::vector<unsigned char>& buffer
		);
		/* Reads a binary file into a string.
		(filePath) The path to the file to load.
		(fileContents)< The string to fill.
		< If the file read was successful */
		bool readTextFileToString(
			const std::string& filePath,
			std::string& fileContents
		);
		/* Overwrite a string to a file.
		(filePath) The path to the file to overwrite.
		(string) The string to write.
		< If the file write was successful */
		bool overwriteStringToFile(
			const std::string& FILE_PATH,
			std::string& string
		);
	};

}