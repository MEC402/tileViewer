#pragma once
#include <GL/glew.h>

class ImageHandler {
public:
	static void InitTextureAtlas(GLuint program);
	static void LoadImageFromPath(const char *path, int face);
	static float TxScalingX(int face);
	static float TxScalingY(int face);

private:
	static void initFaceAtlas(int face, GLuint program, const char *path);

	static int m_maxWidth[6];
	static int m_maxHeight[6];
	static int m_faceWidth[6];
	static int m_faceHeight[6];
	static GLuint m_textures[6];
};