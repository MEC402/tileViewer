//#include "stdafx.h"

#include "ImageHandler.h"
#include "InternetDownload.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_MSC_SECURE_CRT
#include "stb_image_write.h"

#include <mutex>

// Texture names
const char *ImageHandler::m_txUniforms[6] = { "TxFront", "TxBack", "TxRight", "TxLeft", "TxTop", "TxBottom" };
const char ImageHandler::m_faceNames[6] = { 'f', 'b', 'r', 'l', 'u', 'd' };

/* ---------------- Public Functions ---------------- */

ImageHandler::ImageHandler()
{
	m_currentPano = 0;
	m_dumpcount = 0;
	m_stereoLoaded = false;
}

void ImageHandler::InitTextureAtlas(bool stereo, ImageQueue *toRender) 
{
	Decompressed = toRender;
	m_compressed = new ImageQueue();
	
	// 6 for each eye
	glGenTextures(6, m_textures[0]);

	// hardcoded magic
	int maxDepth = 3;

	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, maxDepth, 0);	
	}

	if (m_panoList.size() > 0)
		InitURLs(0, stereo);

	if (stereo) {
		InitStereo();
	}
}

void ImageHandler::InitStereo()
{
	// Try to populate stereo URLs first
	InitStereoURLs();

	// If our texture bindings != 0, we've already initialized the second eye textures
	if (m_textures[1][0] != 0) return;

	glGenTextures(6, m_textures[1]);
	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, 3, 1);
	}

}

void ImageHandler::InitStereoURLs()
{
	if (m_stereoLoaded)
		return;

	m_stereoLoaded = true;
	for (int depth = 0; depth < 4; depth++) {
		int maxDepth = (int)pow(2, depth);
		for (int i = maxDepth - 1; i >= 0; i--) {
			for (int j = maxDepth - 1; j >= 0; j--) {
				for (int face = 0; face < 6; face++) {
					URL url(face, 1);
					sprintf_s(url.buf, m_panoList[m_currentPano].rightAddress.c_str(), depth + 1, m_faceNames[face], i, j);
					m_urls.emplace_back(url);
				}
			}
		}
	}
}

void ImageHandler::InitPanoList(std::string url)
{
	ImageData jsonFile;
	downloadFile(&jsonFile, url);
	try {
		if (jsonFile.data) {
			std::string fileAsString(jsonFile.data, jsonFile.data + jsonFile.dataSize);
			// Base URL is the substring before the last backslash or forward slash
			size_t lastSlashPosition = url.find_last_of("/\\");
			std::string baseURL = url.substr(0, lastSlashPosition);
			m_panoList = parsePanoInfoFile(fileAsString, baseURL);
		}
	}
	catch (const std::exception &exc) {
		fprintf(stderr, "%s\n", exc.what());
	}
}

void ImageHandler::InitURLs(int pano, bool stereo)
{
	std::lock_guard<std::mutex> lock(m_);
	if (m_panoList.size() < 1)
		return;
	m_stereoLoaded = stereo;

	if (!m_urls.empty())
		m_urls.clear();

	for (int depth = 0; depth < 4; depth++) {
		int maxDepth = (int)pow(2, depth);
		for (int i = maxDepth - 1; i >= 0; i--) {
			for (int j = maxDepth - 1; j >= 0; j--) {
				for (int face = 0; face < 6; face++) {
					URL url(face, 0);
					sprintf_s(url.buf, m_panoList[pano].leftAddress.c_str(), depth + 1, m_faceNames[face], i, j);
					m_urls.emplace_back(url);
					if (stereo) {
						URL url(face, 1);
						sprintf_s(url.buf, m_panoList[pano].rightAddress.c_str(), depth + 1, m_faceNames[face], i, j);
						m_urls.emplace_back(url);
					}
				}
			}
		}
	}

	// m_tileDepth is useful for discarding image files for which we already have a better texture
	// Due to logic flow I'm not convinced there's a good place to put such a check, but maybe?
	for (int face = 0; face < 6; face++) {
		for (int row = 0; row < 8; row++) {
			for (int col = 0; col < 8; col++) {
				m_tileDepth[face][row][col] = 0;
			}
		}
	}
}

void ImageHandler::ClearQueues()
{
	std::lock_guard<std::mutex> lock(m_);
	m_compressed->Clear();
	m_compressed->ToggleDiscard();
	m_urls.clear();
}

void ImageHandler::LoadImageData(ImageData *image)
{
	GLenum errCode;
	// TODO: Need to include a given images width/height so we're not hardcoding 512x512
	if (image->data) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[image->eye][image->face]);
		if ((errCode = glGetError()) != GL_NO_ERROR) {
			printf("OPENGL ERROR BINDBUFFER: %s\n", gluErrorString(errCode));
		}

		int* dst = (int*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (dst) {
			std::memcpy(dst, image->data, 512 * 512 * 3);
		}
		if ((errCode = glGetError()) != GL_NO_ERROR) {
			printf("OPENGL ERROR Texture Map Buffer: %s\n", gluErrorString(errCode));
		}
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		glBindTexture(GL_TEXTURE_2D, image->activeTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, image->w_offset, image->h_offset, 512, 512, 
			GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		if ((errCode = glGetError()) != GL_NO_ERROR) {
			printf("OPENGL ERROR Loading Image: %s\n", gluErrorString(errCode));
		}
		
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	else {
		fprintf(stderr, "Error loading image file! No pixel data found\n");
	}
	image->Free();
}

void ImageHandler::LoadQuadImage()
{
	while (true) {
		m_.lock();
		if (m_urls.empty()) {
			m_.unlock();
			return Decompress();
		}

		URL u = m_urls.front();
		m_urls.pop_front();
		m_.unlock();

		ImageData *imageFile = new ImageData{ 0 };
		downloadFile(imageFile, u.buf);

		imageFile->activeTexture = m_textures[u.eye][u.face];
		imageFile->face = u.face;
		imageFile->eye = u.eye;

		m_compressed->Enqueue(imageFile);
	}
}

void ImageHandler::LoadFaceImage(int face, int depth, int eye)
{
	// Basically the same as LoadQuadImage
	const char facename = m_faceNames[face];
	int activeTexture = m_textures[eye][face];
	
	// This seems to work pretty nicely
	int maxDepth = (int)pow(2, depth); // Get the 2^n maximal depth to search for
	std::vector<std::string> urls;

	// Populate our URL vectors in reverse, this helps make loading in images appear smoother (we don't overwrite lower-depth tiles until last)
	for (int i = maxDepth-1; i >= 0; i--) {
		int bufferSize = 256;
		char buf[256];
		for (int j = maxDepth-1; j >= 0; j--) {
			if (eye == 0) {
				sprintf_s(buf, m_panoList[m_currentPano].leftAddress.c_str(), depth + 1, m_faceNames[face], i, j);
			}
			else {
				sprintf_s(buf, m_panoList[m_currentPano].rightAddress.c_str(), depth + 1, m_faceNames[face], i, j);
			}
			urls.push_back(buf);
		}
	}

	// Initialize all our pointers on the heap ahead of time
	// Might be a way to do this on imageFiles declaration?
	ImageData **imageFiles = new ImageData*[maxDepth * maxDepth];
	for (int i = 0; i < maxDepth * maxDepth; i++) {
		imageFiles[i] = new ImageData{ 0 };
	}

	//downloadMultipleFiles(imageFiles, urls.data(), urls.size());
	std::thread t(downloadMultipleFiles, imageFiles, urls.data(), urls.size());
	t.detach();
	//std::thread t(testReuseHandle, imageFiles, urls.data(), urls.size());
	//t.detach();}

	int i = 0;
	int width, height, nrChannels;
	while(i < (maxDepth * maxDepth)) {
		// Check the completion status of each image before trying to load it
		if (imageFiles[i]->complete) {
			// Same as LoadQuadImage, new pointers then free then reassign to avoid memory leaks
			unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)imageFiles[i]->data, imageFiles[i]->dataSize, &width, &height, &nrChannels, 0));

			free(imageFiles[i]->data);

			imageFiles[i]->data = d;
			imageFiles[i]->activeTexture = activeTexture;
			imageFiles[i]->face = face;
			imageFiles[i]->eye = eye;
			imageFiles[i]->w_offset *= width;
			imageFiles[i]->h_offset *= height;

			Decompressed->Enqueue(imageFiles[i]);

			// Reset complete flag to false so we aren't double-counting (This is only set to true in InternetDownloader)
			imageFiles[i]->complete = false;
			i++;
		}
	}
	// No longer need the pointer for our array of ImageData pointers, they're all in the ImageQueue now
	delete[]imageFiles;
}

void ImageHandler::Decompress()
{
	ImageData* imageFile = NULL;
	while (true) {
		m_.lock();

		if (m_urls.empty() && m_compressed->IsEmpty()) {
			m_.unlock();
			return;
		}

		if (!m_compressed->IsEmpty()) {
			imageFile = m_compressed->Dequeue();
		}
		else {
			m_.unlock();
			continue;
		}

		m_.unlock();

		int width, height, nrChannels;
		unsigned char *d = (unsigned char*)stbi_load_from_memory((stbi_uc*)imageFile->data, imageFile->dataSize, &width, &height, &nrChannels, 0);

		free(imageFile->data);

		imageFile->data = d;
		imageFile->w_offset *= width;
		imageFile->h_offset *= height;
		Decompressed->Enqueue(imageFile);
	}
}

// For use after doing a hot-reload on shaders (Or switching between two sets of Texture Atlases)
void ImageHandler::BindTextures(Shader &shader, int eye)
{
	if (m_textures[eye][0] == 0) {
		fprintf(stderr, "No texture loaded for that eye\n");
		return;
	}

	for (int i = 0; i < 6; i++) {
		shader.BindTexture(m_txUniforms[i], i, m_textures[eye][i]);
	}
}

void ImageHandler::WindowDump(int width, int height)
{
	unsigned char* image = (unsigned char*)malloc(width * height * 3 * sizeof(char));

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glReadBuffer(GL_FRONT);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, image);
	std::thread t([](unsigned char* image, int width, int height, int dumpcount)
	{
#ifdef _USE_WIN_H
		// Windows pls
		char buff[FILENAME_MAX];
		char buf[FILENAME_MAX];
		GetCurrentDirectoryA(FILENAME_MAX, buff);
#else
		char buff[1024];
		char buf[1024];
		getcwd(buff, sizeof(buff));
#endif
		std::string cwd(buff);

		sprintf_s(buf, "%s\\Output_%d.png", cwd.c_str(), dumpcount);
		dumpcount++;

		stbi_flip_vertically_on_write(true);
		stbi_write_png(buf, width, height, 3, image, width * 3);

		fprintf(stderr, "Saved image to %s\n", buf);
		free(image);
	}, image, width, height, m_dumpcount);
	t.detach();
}

/* ---------------- Private Functions ---------------- */


void ImageHandler::initFaceAtlas(int face, int depth, int eye)
{
	// TODO: Probably shouldn't hardcode image resolution like this
	int maxWidth = 512 * (int)pow(2, depth);
	int maxHeight = 512 * (int)pow(2, depth);

	glBindTexture(GL_TEXTURE_2D, m_textures[eye][face]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	
	// Pros to Linear: Looks WAY WAY WAY WAY BETTER
	// Cons to Linear: Minor border seams are visible during loading, but largely invisible at max depth
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, maxWidth, maxHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	

	// Init PBOs
	glGenBuffers(1, &m_pbos[eye][face]);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[eye][face]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 512 * 512 * 3, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}