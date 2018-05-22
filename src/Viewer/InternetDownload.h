#include <string>

struct DownloadedFile
{
	unsigned char* data;
	unsigned int dataSize;
	bool complete;
};

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, DownloadedFile *file);
void downloadFile(DownloadedFile* out_file, const std::string url);
void downloadMultipleFiles(DownloadedFile* out_files, const std::string* urls, unsigned int fileCount);