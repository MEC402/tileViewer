#pragma once
#include <GL/glew.h>


class ImageHandler {
public:
	static void InitTextureAtlas(GLuint program);
	static void LoadImageFromPath(const char *path, int face, int depth);
	static void LoadQuadImageFromPath(const char *path, int face, int row, int col, int depth);
	static float TxScalingX(int face);
	static float TxScalingY(int face);

private:
	struct imageData {
		unsigned char *data;
		int width;
		int height;
		int w_offset;
		int h_offset;
	};

	static void initFaceAtlas(int face, int depth, GLuint program);
	static int maxResDepth(const char *path);
	static void threadedImageLoad(const char *path, int depth, const char *facename, int i, int j, imageData *data);

	static int m_maxWidth[6];
	static int m_maxHeight[6];
	static int m_faceWidth[6];
	static int m_faceHeight[6];
	static GLuint m_textures[6];

};