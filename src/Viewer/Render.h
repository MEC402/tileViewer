#ifndef _RENDER_H
#define _RENDER_H
#include <GL\glew.h>

struct Model
{
	GLuint vertexPositionBuffer = 0;
	GLuint vertexUVBuffer = 0;
	GLuint vertexIndexBuffer = 0;
	unsigned int indices = 0;
};

struct Texture
{
	GLuint id = 0;
	unsigned int width = 0;
	unsigned int height = 0;
};

class Render {
public:
	static void CreateQuadModel(Model* model);
	static void CreateCubeModel(Model *model);
	static void DestroyModel(Model* model);
	static void DrawModel(Model model);
	static GLuint CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels = 0);
	static void CreateTexture(Texture *out_texture, unsigned int textureSlot, unsigned int width, unsigned int height, GLenum format, unsigned char *pixels = 0);
	static void DestroyTexture(Texture *texture);
};

#endif // _RENDER_H