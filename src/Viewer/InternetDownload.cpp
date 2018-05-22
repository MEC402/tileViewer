#include "stdafx.h"
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <string>


size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, ImageData *file)
{
	// Files are progressivly written, so we append the new data to any previously downloaded data.
	size_t newByteCount = size * nmemb;
	file->data = (unsigned char*)realloc(file->data, file->dataSize + newByteCount);
	memcpy(file->data + file->dataSize, newBytes, newByteCount);
	file->dataSize += newByteCount;
	return newByteCount;
}

void downloadFile(ImageData *out_file, const std::string url)
{
	*out_file = { 0 };
	CURL* curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, out_file);
	curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	out_file->complete = true;


	std::stringstream ss(url.c_str());
	std::string item;
	std::vector<std::string> tokens;
	while (std::getline(ss, item, '/')) {
		tokens.push_back(item);
	}
	if (tokens.size() > 1) {
		out_file->w_offset = tokens[tokens.size() - 1][0] - 48;
		out_file->h_offset = tokens[tokens.size() - 2][0] - 48;
		out_file->depth = tokens[tokens.size() - 4][0] - 49;
		int magicnumber = 8 / (int)pow(2, out_file->depth);
		out_file->row = 7 - (out_file->h_offset * magicnumber);
		out_file->col = out_file->w_offset * magicnumber;
	}
}

void downloadMultipleFiles(ImageData **out_files, const std::string* urls, unsigned int fileCount)
{
	CURLM* multi = curl_multi_init();
	int transfersRunning = 0;
	int messagesRemaining = 0;
	// Keep track of the handles so we can check which files have finished downloading
	CURL** curlHandles = new CURL*[fileCount];

	// Intialize file requests
	for (unsigned int i = 0; i < fileCount; ++i)
	{
		//out_files[i] = new ImageData{ 0 };

		// Why must C++ overcomplicate everything?
		// Split our URL to pull out our row/column values to use as offsets when inputting to the texture atlas
		std::stringstream ss(urls[i].c_str());
		std::string item;
		std::vector<std::string> tokens;
		while (std::getline(ss, item, '/')) {
			tokens.push_back(item);
		}
		// Grab our last two values, subtracting 48 to reduce from ASCII char value to actual int value
		out_files[i]->w_offset = tokens[tokens.size() - 1][0] - 48;
		out_files[i]->h_offset = tokens[tokens.size() - 2][0] - 48;
		out_files[i]->depth = tokens[tokens.size() - 4][0] - 49;
		int magicnumber = 8 / (int)pow(2, out_files[i]->depth);
		out_files[i]->row = out_files[i]->h_offset * magicnumber;
		out_files[i]->col = out_files[i]->w_offset * magicnumber;


		CURL *eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_URL, urls[i].c_str());
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, &(*out_files[i]));
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
}