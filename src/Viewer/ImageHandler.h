#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <algorithm>
#include <chrono>
#include <deque>
#include <fstream>
#include <string>
#include <thread>
#include <vector>

#ifdef WIN32
#define _USE_WIN_H
#include <windows.h>
#else
#include <unistd.h>
#endif

#include "Image.h"
#include "ImageHandler.h"
#include "ImageQueue.h"
#include "PanoInfo.h"
#include "ThreadPool.hpp"
#include "SafeQueue.h"
#include "Shader.h"
#include "Shared.h"

struct PanoInfo;

class ImageHandler {

public:
	std::vector<PanoInfo> m_panoList;
	int m_currentPano;
	ImageQueue *Decompressed;

	void InitTextureAtlas(bool stereo, ImageQueue *toRender, Shader &shader);
	void InitStereo(Shader &shader);
	void InitStereoURLs(void);
	bool InitPanoList(std::string url);
	void InitURLs(int pano, bool stereo);

	void LoadImageData(ImageData *image);
	void BindTextures(Shader &shader, int eye);
	void Screenshot(int width, int height);

	void ClearQueues(void);
	void LoadQuadImage(void);
	void Decompress(void);

	ImageHandler();

private:
	void initFaceAtlas(int face, int depth, int eye, Shader &shader);

	struct URL {
		char buf[256];
		int face;
		int eye;
		URL(int f = 0, int e = 0) :face(f), eye(e) {}
	};

	std::mutex m_;
	GLuint m_textures[2][6];
	GLuint m_pbos[2][6];
	static const char *m_txUniforms[6];
	static const char m_faceNames[6];

	//std::deque<URL> m_urls;
	SafeQueue<URL> *m_urls;
	SafeQueue<ImageData*> *m_compressed;
	//ImageQueue *m_compressed;

	int m_tileDepth[6][8][8];

	int m_dumpcount;
	bool m_stereoLoaded;

	std::chrono::high_resolution_clock::time_point t1;
};

#endif