#ifndef _RENDER_H
#define _RENDER_H
#include <GL\glew.h>

struct Model
{
	GLuint vertexPositionbuffer;
	GLuint vertexUVBuffer;
	unsigned int indices;
};

void createModelFromQuad(Model* model);
void renderModel(Model model);

GLuint createTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels = 0);

#endif // _RENDER_H