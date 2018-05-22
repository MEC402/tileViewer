#include <mutex>
#include <queue>
#include "Image.h"

class ImageQueue {

public:
	static bool IsEmpty();
	static void Enqueue(ImageData file);
	static ImageData Dequeue();
	

private:
	static std::mutex mutex_;
	static std::queue<ImageData> queue_;

};