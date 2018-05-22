#include "stdafx.h"

std::mutex ImageQueue::mutex_;
std::queue<ImageHandler::ImageData> ImageQueue::queue_;

bool ImageQueue::IsEmpty() 
{
	mutex_.lock();
	bool isEmpty = queue_.empty();
	mutex_.unlock();

	return isEmpty;
}

void ImageQueue::Enqueue(ImageHandler::ImageData file)
{
	mutex_.lock();
	queue_.push(file);
	mutex_.unlock();
}

ImageHandler::ImageData ImageQueue::Dequeue()
{
	mutex_.lock();
	ImageHandler::ImageData file = queue_.front();
	queue_.pop();
	mutex_.unlock();
	return file;
}