#include "IOManager.h"
#include <fstream>


bool IOManager::readBinaryFileToBuffer(
	const std::string& FILE_PATH,
	std::vector<unsigned char>& buffer
) {
	// Read file as binary
	std::ifstream file(FILE_PATH, std::ios::binary);
	if (file.fail()) {
		perror(FILE_PATH.c_str());
		return false;
	}
	// Find file length
	file.seekg(0, std::ios::end);
	int fileSize(static_cast<int>(file.tellg()));
	file.seekg(0, std::ios::beg);
	fileSize -= static_cast<int>(file.tellg());
	// Fill buffer
	buffer.resize(fileSize);
	file.read((char *)&(buffer[0]), fileSize);
	// Close file
	file.close();
	return true;
}
bool IOManager::readTextFileToString(
	const std::string& FILE_PATH,
	std::string& fileContents
) {
	// Open the file
	std::ifstream file(FILE_PATH);
	if (file.fail()) {
		perror(FILE_PATH.c_str());
		return false;
	}
	// Copy file
	fileContents = "";
	std::string line;
	while (std::getline(file, line)) { fileContents += line + "\n"; }
	// Close file
	file.close();
	return true;
}
bool IOManager::overwriteStringToFile(
	const std::string& FILE_PATH,
	std::string& string
) {
	// Open the file
	std::ofstream file(FILE_PATH);
	if (file.fail()) {
		perror(FILE_PATH.c_str());
		return false;
	}
	// Write file
	file << string;
	// Close file
	file.close();
	return true;
}