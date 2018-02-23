#pragma once
#include <vector>
#include <string>


/* Used to load input files as a static class.
*/
class IOManager {
	
public:

	/* Reads a binary file into a vector of chars.
	@param filePath The path to the file to load.
	@param buffer The vector to fill.
	@return  If the file read was successful
	*/
	static bool readBinaryFileToBuffer(const std::string& filePath, std::vector<unsigned char>& buffer);
		
	/* Reads a binary file into a string.
	@param filePath The path to the file to load.
	@param fileContents The string to fill.
	@return If the file read was successful
	*/
	static bool readTextFileToString(const std::string& filePath, std::string& fileContents);
		
	/* Overwrite a string to a file.
	@param filePath The path to the file to overwrite.
	@param string The string to write.
	@return If the file write was successful
	*/
	static bool overwriteStringToFile(const std::string& FILE_PATH, std::string& string);

private:
		
	/* Private constructor because this is a static class.
	*/
	inline IOManager() {};
};