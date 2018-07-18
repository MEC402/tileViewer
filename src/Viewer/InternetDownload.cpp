//#include "stdafx.h"
#include "InternetDownload.h"

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, ImageData *file)
{
	// Files are progressivly written, so we append the new data to any previously downloaded data.
	size_t newByteCount = size * nmemb;
	file->data = (unsigned char*)realloc(file->data, file->dataSize + newByteCount);
	memcpy(file->data + file->dataSize, newBytes, newByteCount);
	file->dataSize += newByteCount;
	return newByteCount;
}

void populateImageData(ImageData *out_file, const char *url)
{
	std::stringstream ss(url);
	std::string item;
	std::vector<std::string> tokens;
	while (std::getline(ss, item, '/')) {
		tokens.push_back(item);
	}

	// Prevent crashes on loading malformed JSON
	if (tokens.size() > 1) {
		// Grab our last two values, subtracting 48 to reduce from ASCII char value to actual int value
		out_file->w_offset = tokens[tokens.size() - 1][0] - 48;
		out_file->h_offset = tokens[tokens.size() - 2][0] - 48;
		// Depth is currently stored on disk with 1 indexing, so subtract 49 to make it 0 indexed
		out_file->depth = tokens[tokens.size() - 4][0] - 49;
		// Same math as CubePoints::QuadNextDepth (again)
		int magicnumber = 8 / (int)pow(2, out_file->depth);
		out_file->row = 7 - (out_file->h_offset * magicnumber);
		out_file->col = out_file->w_offset * magicnumber;
	}
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

	if (url.substr(url.find_last_of(".") + 1) != "json")
		populateImageData(out_file, url.c_str());
}

std::vector<ImageData> downloadMultipleFiles(const std::string *urls, unsigned int fileCount)
{
	std::vector<ImageData> out_files(fileCount);
	CURLM *multi = curl_multi_init();
	int transfersRunning = 0;
	int messagesRemaining = 0;

	// Intialize file requests
	for (unsigned int i = 0; i < fileCount; ++i)
	{
		out_files[i] = { 0 };
		CURL *eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_URL, urls[i].c_str());
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, &(out_files[i]));
		curl_multi_add_handle(multi, eh);
	}

	curl_multi_perform(multi, &transfersRunning);

	// Progressively download the files
	do {
		const int maxWaitTimeMiliseconds = 10 * 10000;
		int numfds = 0;
		int result = curl_multi_wait(multi, NULL, 0, maxWaitTimeMiliseconds, &numfds);
		curl_multi_perform(multi, &transfersRunning);
	} while (transfersRunning != 0);

	curl_multi_cleanup(multi);
	return out_files;
}