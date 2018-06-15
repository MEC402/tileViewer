#include "Image.h"
#include <mutex>
#include <deque>

template <class T>
class SafeQueue {
public:
	SafeQueue()
	{
		discard = false;
	}

	~SafeQueue() = default;

	//template<>
	//~SafeQueue<ImageData>() = default;

	void Clear()
	{
		ToggleDiscard();
		std::lock_guard<std::mutex> lock(mutex_);
		while (!queue_.empty()) {
			T t = queue_.front();
			queue_.pop_front();
		}
	}

	T Dequeue()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
			return NULL;
		T t = queue_.front();
		queue_.pop_front();
		return t;
	}

	void Enqueue(T t)
	{
		if (!discard) {
			std::lock_guard<std::mutex> lock(mutex_);
			queue_.push_back(t);
		}
	}

	bool IsEmpty()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.empty();
	}

	int Size()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		return queue_.size();
	}

	void ToggleDiscard()
	{
		std::lock_guard<std::mutex> lock(mutex_);
		discard = !discard;
	}

	

private:
	std::mutex mutex_;
	bool discard;
	std::deque<T> queue_;
};

template <>
void SafeQueue<ImageData*>::Clear()
{
	ToggleDiscard();
	std::lock_guard<std::mutex> lock(mutex_);
	while (!queue_.empty()) {
		ImageData *i = queue_.front();
		queue_.pop_front();
		i->Free();
	}
}

template <>
void SafeQueue<ImageData*>::Enqueue(ImageData *i)
{
	if (discard) {
		i->Free();
	}
	else {
		std::lock_guard<std::mutex> lock(mutex_);
		queue_.push_back(i);
	}
}