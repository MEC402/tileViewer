#ifndef IMAGEQUEUE_H
#define IMAGEQUEUE_H

#include <mutex>
#include <queue>
#include "Image.h"

class ImageQueue {

public:

	~ImageQueue() = default;

	void Clear();
	ImageData* Dequeue();
	void Enqueue(ImageData *file);
	bool IsEmpty();
	int Size();
	void ToggleDiscard();

private:
	std::mutex mutex_;
	std::queue<ImageData*> queue_;
	bool discard; // Discard anything that comes in while we're clearing a queue

};

#endif