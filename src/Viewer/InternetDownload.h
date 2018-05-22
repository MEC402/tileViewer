#include <string>

//struct DownloadedFile
//{
//	unsigned char *data;
//	unsigned int dataSize;
//	int xOffset;
//	int yOffset;
//	bool complete;
//};

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, ImageData *file);
void downloadFile(ImageData *out_file, const std::string url);
void downloadMultipleFiles(ImageData *out_files, const std::string* urls, unsigned int fileCount);