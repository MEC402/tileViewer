#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <algorithm>
#include <chrono>
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
#include "InternetDownload.h"
#include "PanoInfo.h"
#include "ThreadPool.hpp"

struct PanoInfo;

class ImageHandler {

public:
	static std::vector<PanoInfo> m_panoList;
	static int m_currentPano;

	static void InitTextureAtlas(GLuint program, bool stereo);
	static void InitStereo(GLuint program);
	static void InitPanoListFromOnlineFile(std::string url);
	static void LoadImageData(ImageData *image);
	static void LoadFaceImage(int face, int depth, int eye);
	static void LoadQuadImage(int face, int row, int col, int depth, int eye);
	static void RebindTextures(GLuint program, int eye);
	static void WindowDump(int width, int height);

private:
	static void initFaceAtlas(int face, int depth, int eye, GLuint program);

	// WIN32 API calls for traversing a directory (no longer necessary?)
#ifdef _USE_WIN_H
	static int maxResDepth(const char *path);
#endif

	static std::mutex m_;
	static GLuint m_textures[2][6];
	static GLuint m_pbos[2][6];
	static const char *m_txUniforms[6];
	static const char m_faceNames[6];

	static int m_tileDepth[6][8][8];
	//static std::vector<ImageData> m_imageData;
	static int m_dumpcount;
};

#endif