#include "stdafx.h"


//TODO: Should we hold onto pointers instead?  This might be storing copies of our data

std::mutex ImageQueue::mutex_;
std::queue<ImageData*> ImageQueue::queue_;

bool ImageQueue::IsEmpty() 
{
	mutex_.lock();
	bool isEmpty = queue_.empty();
	mutex_.unlock();

	return isEmpty;
}

void ImageQueue::Enqueue(ImageData *file)
{
	mutex_.lock();
	queue_.push(file);
	mutex_.unlock();
}

ImageData* ImageQueue::Dequeue()
{
	mutex_.lock();
	ImageData *file = queue_.front();
	queue_.pop();
	mutex_.unlock();
	return file;
}