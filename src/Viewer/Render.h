#ifndef _RENDER_H
#define _RENDER_H
#include <GL\glew.h>

struct Model
{
	GLuint vertexPositionbuffer;
	GLuint vertexUVBuffer;
	unsigned int indices;
};

void Render_CreateQuadModel(Model* model);
void Render_DrawModel(Model model);

GLuint Render_CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels = 0);

#endif // _RENDER_H