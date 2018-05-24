#pragma once
#include <GL/glew.h>
#include "Image.h"
#include "InternetDownload.h"
#include "ThreadPool.hpp"

struct PanoInfo;

class ImageHandler {

public:

	static void InitTextureAtlas(GLuint program);
	static void InitPanoListFromOnlineFile(std::string url);
	static void LoadImageData(ImageData *image);
	static void LoadFaceImage(int face, int depth);
	static void LoadQuadImage(int face, int row, int col, int depth);
	static void RebindTextures(GLuint program);

private:
	static void initFaceAtlas(int face, int depth, GLuint program);
	static int maxResDepth(const char *path);

	static GLuint m_textures[6];
	static const char *m_txUniforms[6];
	static const char m_faceNames[6];

	static int m_tileDepth[6][8][8];
	static std::vector<PanoInfo> m_panoList;
	static std::vector<ImageData> m_imageData;
};