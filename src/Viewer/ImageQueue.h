#ifndef IMAGEQUEUE_H
#define IMAGEQUEUE_H

#include <mutex>
#include <queue>
#include "Image.h"

class ImageQueue {

public:
	static void Clear();
	static ImageData* Dequeue();
	static void Enqueue(ImageData *file);
	static bool IsEmpty();
	static int Size();
	static void ToggleDiscard();

private:
	static std::mutex mutex_;
	static std::queue<ImageData*> queue_;
	static bool discard; // Discard anything that comes in while we're clearing a queue

};

#endif