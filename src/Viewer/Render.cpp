#include "Render.h"
#include "Shared.h"
#define _USE_MATH_DEFINES
#include <cmath>

void Render::CreateQuadModel(Model* model)
{
	const float positions[] = {
		// Triangle 1
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
		// Triangle 2
		1, 1, 0,
		-1, 1, 0,
		-1, -1, 0
	};

	const float uvs[] = {
		// Triangle 1
		0, 0,
		1, 0,
		1, 1,
		// Triangle 2
		1, 1,
		0, 1,
		0, 0
	};

	glGenBuffers(1, &model->vertexPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

	model->indices = 2 * 3;
}

void Render::CreateCubeModel(Model *model)
{
	const GLuint indices[] = {
		0, 1, 2, 0, 2, 3,
		4, 5, 6, 4, 6, 7,
		8, 9, 10, 8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};

	const GLfloat vertices[] = { 
		 1, 1, 1,  -1, 1, 1,  -1,-1, 1,   1,-1, 1, 
		 1, 1, 1,   1,-1, 1,   1,-1,-1,   1, 1,-1, 
		 1, 1, 1,   1, 1,-1,  -1, 1,-1,  -1, 1, 1, 
		-1, 1, 1,  -1, 1,-1,  -1,-1,-1,  -1,-1, 1,
		-1,-1,-1,   1,-1,-1,   1,-1, 1,  -1,-1, 1,
		 1,-1,-1,  -1,-1,-1,  -1, 1,-1,   1, 1,-1 
	};

	const GLfloat uvs[] = {
		0.000059f, 1.0f - 0.000004f,
		0.000103f, 1.0f - 0.336048f,
		0.335973f, 1.0f - 0.335903f,
		1.000023f, 1.0f - 0.000013f,
		0.667979f, 1.0f - 0.335851f,
		0.999958f, 1.0f - 0.336064f,
		0.667979f, 1.0f - 0.335851f,
		0.336024f, 1.0f - 0.671877f,
		0.667969f, 1.0f - 0.671889f,
		1.000023f, 1.0f - 0.000013f,
		0.668104f, 1.0f - 0.000013f,
		0.667979f, 1.0f - 0.335851f,
		0.000059f, 1.0f - 0.000004f,
		0.335973f, 1.0f - 0.335903f,
		0.336098f, 1.0f - 0.000071f,
		0.667979f, 1.0f - 0.335851f,
		0.335973f, 1.0f - 0.335903f,
		0.336024f, 1.0f - 0.671877f,
		1.000004f, 1.0f - 0.671847f,
		0.999958f, 1.0f - 0.336064f,
		0.667979f, 1.0f - 0.335851f,
		0.668104f, 1.0f - 0.000013f,
		0.335973f, 1.0f - 0.335903f,
		0.667979f, 1.0f - 0.335851f,
		0.335973f, 1.0f - 0.335903f,
		0.668104f, 1.0f - 0.000013f,
		0.336098f, 1.0f - 0.000071f,
		0.000103f, 1.0f - 0.336048f,
		0.000004f, 1.0f - 0.671870f,
		0.336024f, 1.0f - 0.671877f,
		0.000103f, 1.0f - 0.336048f,
		0.336024f, 1.0f - 0.671877f,
		0.335973f, 1.0f - 0.335903f,
		0.667969f, 1.0f - 0.671889f,
		1.000004f, 1.0f - 0.671847f,
		0.667979f, 1.0f - 0.335851f
	};

	glGenBuffers(1, &model->vertexPositionBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexIndexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->vertexIndexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);
}

void Render::DestroyModel(Model* model)
{
	glDeleteBuffers(1, &model->vertexPositionBuffer);
	glDeleteBuffers(1, &model->vertexUVBuffer);
	glDeleteBuffers(1, &model->vertexIndexBuffer);
	*model = { 0 };
}

void Render::DrawModel(Model model)
{

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexPositionBuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, model.indices);
	PRINT_GL_ERRORS;
}

GLuint Render::CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels)
{
	GLuint texture;

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
	PRINT_GL_ERRORS;

	return texture;
}

void Render::CreateTexture(Texture* out_texture, unsigned int textureSlot, unsigned int width, unsigned int height, GLenum format, unsigned char *pixels)
{
	out_texture->id = CreateTexture(textureSlot, width, height, format, pixels);
	out_texture->width = width;
	out_texture->height = height;
}

void Render::DestroyTexture(Texture *texture)
{
	glDeleteTextures(1, &texture->id);
	*texture = { 0 };
}