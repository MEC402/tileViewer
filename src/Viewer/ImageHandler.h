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

struct PanoInfo;

class ImageHandler {

public:
	std::vector<PanoInfo> m_panoList;
	int m_currentPano;
	ImageQueue *Decompressed;

	void InitTextureAtlas(GLuint program, bool stereo, ImageQueue *toRender);
	void InitStereo(GLuint program);
	void InitStereoURLs(void);
	void InitPanoList(std::string url);
	void InitURLs(int pano, bool stereo);

	void LoadImageData(ImageData *image);
	void bindTextures(GLuint program, int eye);
	void WindowDump(int width, int height);

	void ClearQueues(void);
	void LoadFaceImage(int face, int depth, int eye);
	void LoadQuadImage(void);
	void Decompress(void);

	ImageHandler();

private:
	void initFaceAtlas(int face, int depth, int eye, GLuint program);

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

	std::deque<URL> m_urls;
	ImageQueue *m_compressed;

	int m_tileDepth[6][8][8];

	int m_dumpcount;
	bool m_stereoLoaded;

	std::chrono::high_resolution_clock::time_point t1;
};

#endif