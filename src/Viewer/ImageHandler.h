#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <algorithm>
#include <chrono>
#include <string>
#include <thread>
#include <vector>
#include <windows.h>

#include "Image.h"
#include "ImageHandler.h"
#include "ImageQueue.h"
#include "InternetDownload.h"
#include "PanoInfo.h"
#include "ThreadPool.hpp"

struct PanoInfo;

class ImageHandler {

public:

	static void InitTextureAtlas(GLuint program);
	static void InitPanoListFromOnlineFile(std::string url);
	static void LoadImageData(ImageData *image);
	static void LoadFaceImage(int face, int depth, int eye);
	static void LoadQuadImage(int face, int row, int col, int depth, int eye);
	static void RebindTextures(GLuint program, int eye);

private:
	static void initFaceAtlas(int face, int depth, int eye, GLuint program);
	static int maxResDepth(const char *path);

	static GLuint m_textures[2][6];
	static const char *m_txUniforms[6];
	static const char m_faceNames[6];

	static int m_tileDepth[6][8][8];
	static std::vector<PanoInfo> m_panoList;
	static std::vector<ImageData> m_imageData;
};

#endif