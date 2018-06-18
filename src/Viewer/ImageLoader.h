#include "SafeQueue.h"
#include "Image.h"

// Multithreadable image loader
class ImageLoader
{
public:
	void LoadQuadImage(void);
	void Decompress(void);

private:
	struct URL {
		char buf[256];
		int face;
		int eye;
		URL(int f = 0, int e = 0) :face(f), eye(e) {}
	};

	std::mutex m_mutex;
	SafeQueue<URL> m_urls;
	SafeQueue<ImageData*> m_compressed;
	SafeQueue<ImageData*> m_decompressed;
};