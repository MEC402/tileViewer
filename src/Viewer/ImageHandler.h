#pragma once
#include <GL/glew.h>
#include "InternetDownload.h"

struct PanoInfo;

class ImageHandler {
public:

	static void InitTextureAtlas(GLuint program);
	static void InitPanoListFromOnlineFile(std::string url);
	static void LoadFaceImage(int face, int depth);
	static void LoadQuadImage(int face, int row, int col, int depth);
	static float TxScalingX(int face);
	static float TxScalingY(int face);
	static void RebindTextures(GLuint program);

private:
	struct ImageData {
		DownloadedFile file;
		int w_offset;
		int h_offset;
	};

	static void initFaceAtlas(int face, int depth, GLuint program);
	static int maxResDepth(const char *path);
	static void threadedImageLoad(const char *path, int depth, const char *facename, int i, int j, ImageData *data);

	static int m_maxWidth[6];
	static int m_maxHeight[6];
	static int m_faceWidth[6];
	static int m_faceHeight[6];
	static GLuint m_textures[6];
	static const char *m_txUniforms[6];
	static const char m_faceNames[6];

	static int m_tileDepth[6][8][8];
	static std::vector<PanoInfo> m_panoList;
	static std::vector<ImageData> m_imageData;
};