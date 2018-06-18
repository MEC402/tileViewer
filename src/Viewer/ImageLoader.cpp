#include "ImageLoader.h"
#include "InternetDownload.h"
#include "Shared.h"
#include "stb_image.h"


void ImageLoader::LoadQuadImage()
{
	while (true) {
		m_mutex.lock();
		if (m_urls.IsEmpty()) {
			m_mutex.unlock();
			Decompress();
			return;
		}
		URL u = m_urls.Dequeue();
		m_mutex.unlock();

		ImageData *imageFile = new ImageData{ 0 };
		downloadFile(imageFile, u.buf);

		imageFile->face = u.face;
		imageFile->eye = u.eye;

		m_compressed.Enqueue(imageFile);
	}
}

void ImageLoader::Decompress()
{
	ImageData* imageFile = NULL;
	while (true) {

		if (m_urls.IsEmpty() && m_compressed.IsEmpty())
			return;

		if ((imageFile = m_compressed.Dequeue()) == NULL)
			continue;


		int width, height, nrChannels;
		unsigned char *d = (unsigned char*)stbi_load_from_memory((stbi_uc*)imageFile->data, imageFile->dataSize, &width, &height, &nrChannels, 0);

		free(imageFile->data);

		imageFile->data = d;
		imageFile->w_offset *= width;
		imageFile->h_offset *= height;
		imageFile->colorChannels = nrChannels;
		imageFile->width = width;
		imageFile->height = height;
		m_decompressed.Enqueue(imageFile);
	}
}