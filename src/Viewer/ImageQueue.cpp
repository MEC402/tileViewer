//#include "stdafx.h"
#include "ImageQueue.h"

std::mutex ImageQueue::mutex_;
std::queue<ImageData*> ImageQueue::queue_;

bool ImageQueue::IsEmpty() 
{
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.empty();
}

void ImageQueue::Enqueue(ImageData *file)
{
	std::lock_guard<std::mutex> lock(mutex_);
	queue_.push(file);
}

ImageData* ImageQueue::Dequeue()
{
	std::lock_guard<std::mutex> lock(mutex_);
	ImageData *file = queue_.front();
	queue_.pop();
	return file;
}

void ImageQueue::Clear()
{
	std::lock_guard<std::mutex> lock(mutex_);
	while (!queue_.empty()) {
		ImageData *i = queue_.front();
		queue_.pop();
		free(i->data);
		free(i);
	}
}