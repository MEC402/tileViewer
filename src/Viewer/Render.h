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

namespace Render {
	void CreateQuadModel(Model* model);
	void CreateCubeModel(Model *model);
	void DestroyModel(Model* model);
	void DrawModel(Model model);
	GLuint CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels = 0);
	void CreateTexture(Texture *out_texture, unsigned int textureSlot, unsigned int width, unsigned int height, GLenum format, unsigned char *pixels = 0);
	void DestroyTexture(Texture *texture);
};

#endif // _RENDER_H