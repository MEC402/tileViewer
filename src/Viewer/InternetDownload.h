#ifndef INTERNETDOWNLOAD_H
#define INTERNETDOWNLOAD_H

#include <string>
#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <vector>

#include "Image.h"

size_t downloadFileWriterCallback(void *newBytes, size_t size, size_t nmemb, ImageData *file);
void populateImageData(ImageData *out_file, const char *url);
void downloadFile(ImageData *out_file, const std::string url);
void downloadMultipleFiles(ImageData **out_files, const std::string *urls, unsigned int fileCount);

#endif