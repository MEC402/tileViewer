#ifndef _RENDER_H
#define _RENDER_H
#include <GL\glew.h>

struct Model
{
	GLuint vertexPositionbuffer = 0;
	GLuint vertexUVBuffer = 0;
	unsigned int indices = 0;
};

struct Texture
{
	GLuint id = 0;
	unsigned int width = 0;
	unsigned int height = 0;
};

void Render_CreateQuadModel(Model* model);
void Render_CreateCurvedQuadModel(Model* model);
void Render_DrawModel(Model model);

GLuint Render_CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels = 0);
void Render_CreateTexture(Texture *out_texture, unsigned int textureSlot, unsigned int width, unsigned int height, GLenum format, unsigned char *pixels = 0);

#endif // _RENDER_H