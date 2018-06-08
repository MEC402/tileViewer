//#include "stdafx.h"
#include "ImageQueue.h"

std::mutex mutex_;
std::queue<ImageData*> queue_;
bool discard = false;

void ImageQueue::Clear()
{
	ToggleDiscard();
	std::lock_guard<std::mutex> lock(mutex_);
	while (!queue_.empty()) {
		ImageData *i = queue_.front();
		queue_.pop();
		i->Free();
	}
}
ImageData* ImageQueue::Dequeue()
{
	std::lock_guard<std::mutex> lock(mutex_);
	ImageData *file = queue_.front();
	queue_.pop();
	return file;
}

void ImageQueue::Enqueue(ImageData *file)
{
	if (discard) {
		file->Free();
	}
	else {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push(file);
	}
}

bool ImageQueue::IsEmpty() 
{
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.empty();
}

int ImageQueue::Size()
{
	std::lock_guard<std::mutex> lock(mutex_);
	return queue_.size();
}

void ImageQueue::ToggleDiscard()
{
	std::lock_guard<std::mutex> lock(mutex_);
	discard = !discard;
}