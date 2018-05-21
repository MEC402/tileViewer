#include <curl/curl.h>
#include <string>

struct DownloadedFile
{
	char* data;
	unsigned int dataSize;
	bool complete;
};

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, DownloadedFile *file)
{
	// Files are progressivly written, so we append the new data to any previously downloaded data.
	size_t newByteCount = size * nmemb;
	file->data = (char*)realloc(file->data, file->dataSize + newByteCount);
	memcpy(file->data + file->dataSize, newBytes, newByteCount);
	file->dataSize += newByteCount;
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

void downloadMultipleFiles(DownloadedFile* out_files, const std::string* urls, unsigned int fileCount)
{
	CURLM* multi = curl_multi_init();
	int transfersRunning = 0;
	int messagesRemaining = 0;
	// Keep track of the handles so we can check which files have finished downloading
	CURL** curlHandles = new CURL*[fileCount];

	// Intialize file requests
	for (unsigned int i = 0; i < fileCount; ++i)
	{
		out_files[i] = { 0 };
		CURL *eh = curl_easy_init();
		curl_easy_setopt(eh, CURLOPT_URL, urls[i].c_str());
		curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, downloadFileWriterCallback);
		curl_easy_setopt(eh, CURLOPT_WRITEDATA, &out_files[i]);
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
						out_files[i].complete = true;
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