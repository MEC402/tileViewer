#pragma once
#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "rapidjson/document.h"


struct PanoInfo
{
	std::string id;
	std::string displayName;
	std::string leftAddress;
	std::string rightAddress;
	std::string thumbAddress;
};

struct DownloadedFile
{
	char* data;
	unsigned int dataSize;
	bool complete;
};

struct DownloadedImage
{
	DownloadedFile file;
	unsigned char* decompressedPixels;
	unsigned int row;
	unsigned int column;
	unsigned int resolutionLevel;
	unsigned int eyeIndex;
	unsigned int faceIndex;
	int width;
	int height;
};

std::string replaceSubstring(std::string source, std::string find, std::string replacement)
{
	int position = source.find(find);
	if (position != source.npos) {
		source.replace(position, replacement.size(), replacement);
	}
	return source;
}

std::vector<PanoInfo> parsePanoInfoFile(std::string jsonFileRawText, std::string baseImageURL) {
	std::vector<PanoInfo> panoInfoList;
	rapidjson::Document jsonDocument;
	jsonDocument.Parse(jsonFileRawText.c_str());
	if (!jsonDocument.HasParseError() && jsonDocument.HasMember("panos"))
	{
		rapidjson::Value& panosArray = jsonDocument["panos"];
		// Iterate through the array of panos
		for (unsigned int i = 0; i < panosArray.Size(); ++i)
		{
			PanoInfo pano;
			if (panosArray[i].HasMember("id")
				&& panosArray[i].HasMember("name")
				&& panosArray[i].HasMember("left")
				&& panosArray[i].HasMember("right")
				&& panosArray[i].HasMember("thumb"))
			{
				pano.id = panosArray[i]["id"].GetString();
				pano.displayName = panosArray[i]["name"].GetString();
				pano.leftAddress = baseImageURL + '/' + panosArray[i]["left"].GetString();
				pano.leftAddress = replaceSubstring(pano.leftAddress, "%d/%d/%d/%d", "%d/%c/%d/%d");
				pano.rightAddress = baseImageURL + '/' + panosArray[i]["right"].GetString();
				pano.rightAddress = replaceSubstring(pano.rightAddress, "%d/%d/%d/%d", "%d/%c/%d/%d");
				pano.thumbAddress = baseImageURL + '/' + panosArray[i]["thumb"].GetString();
			}
			else
			{
				printf("Incomplete data for pano no. %d\n", i);
			}
			panoInfoList.push_back(pano);
		}
	}
	else
	{
		printf("Could not read the pano JSON file");
	}
	return panoInfoList;
}

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, DownloadedFile *file)
{
	// Files are progressivly written, so we append the new data to any previously downloaded data.
	size_t newByteCount = size * nmemb;
	file->data = (char*)realloc(file->data, file->dataSize + newByteCount);
	memcpy(file->data + file->dataSize, newBytes, newByteCount);
	file->dataSize += (unsigned int)newByteCount;
	return newByteCount;
}

void downloadFile(DownloadedFile* out_file, const std::string url)
{
	*out_file = { 0 };
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	out_file->complete = true;
}

void downloadMultipleFiles(DownloadedFile** out_files, const std::string* urls, unsigned int fileCount)
{
	CURLM* multi = curl_multi_init();
	int transfersRunning = 0;
	int messagesRemaining = 0;
	// Keep track of the handles so we can check which files have finished downloading
	CURL** curlHandles = new CURL*[fileCount];

	// Intialize file requests
	for (unsigned int i = 0; i < fileCount; ++i)
	{
		*(out_files[i]) = { 0 };
		CURL *eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_URL, urls[i].c_str());
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, out_files[i]);
		curl_multi_add_handle(multi, eh);
		curlHandles[i] = eh;
	}

	// Progressively download the files
	curl_multi_perform(multi, &transfersRunning);
	do {
		const int maxWaitTimeMiliseconds = 10 * 10000;
		int numfds = 0;
		int result = curl_multi_wait(multi, NULL, 0, maxWaitTimeMiliseconds, &numfds);
		curl_multi_perform(multi, &transfersRunning);

		// Check if a file has finished downloading
		CURLMsg* m = 0;
		do {
			int msgq = 0;
			m = curl_multi_info_read(multi, &msgq);
			if (m && (m->msg == CURLMSG_DONE)) {
				CURL *e = m->easy_handle;
				// Search the curlHandles array to figure out which file finished
				for (unsigned int i = 0; i < fileCount; ++i)
				{
					if (curlHandles[i] == e) {
						out_files[i]->complete = true;
					}
				}
				curl_multi_remove_handle(multi, e);
				curl_easy_cleanup(e);
			}
		} while (m);

	} while (transfersRunning != 0);

	curl_multi_cleanup(multi);
	delete[] curlHandles;
}

std::vector<PanoInfo> InitPanoListFromOnlineFile(std::string url)
{
	std::vector<PanoInfo> panoList;
	DownloadedFile jsonFile;
	downloadFile(&jsonFile, url);
	if (jsonFile.data) {
		std::string fileAsString(jsonFile.data, jsonFile.data + jsonFile.dataSize);
		// Base URL is the substring before the last backslash or forward slash
		size_t lastSlashPosition = url.find_last_of("/\\");
		std::string baseURL = url.substr(0, lastSlashPosition);
		panoList = parsePanoInfoFile(fileAsString, baseURL);
	}
	return panoList;
}