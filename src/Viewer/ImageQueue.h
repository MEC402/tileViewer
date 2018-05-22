#include <mutex>
#include <queue>
#include "ImageHandler.h"

class ImageQueue {

public:
	static bool IsEmpty();
	static void Enqueue(ImageHandler::ImageData file);
	static ImageHandler::ImageData Dequeue();
	

private:
	static std::mutex mutex_;
	static std::queue<ImageHandler::ImageData> queue_;

};