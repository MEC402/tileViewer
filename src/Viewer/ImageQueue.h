#ifndef IMAGEQUEUE_H
#define IMAGEQUEUE_H

#include <mutex>
#include <queue>
#include "Image.h"

class ImageQueue {

public:
	static bool IsEmpty();
	static void Enqueue(ImageData *file);
	static ImageData* Dequeue();
	static void Clear();

private:
	static std::mutex mutex_;
	static std::queue<ImageData*> queue_;

};

#endif