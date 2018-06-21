#include "Render.h"
#include "Shared.h"


void Render_CreateQuadModel(Model* model)
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

	glGenBuffers(1, &model->vertexPositionbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexPositionbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

	model->indices = 2 * 3;
}

void Render_CreateCurvedQuadModel(Model* model)
{
	struct vertexPosition { float x, y, z; };
	struct vertexTexCoords { float u, v; };

	const vertexPosition p[] = {
		{ 0.578252, 0.578252, -0.578252 },
		{ 0.578252, -0.578252, -0.578252 },
		{ -0.578252, 0.578252, -0.578252 },
		{ -0.578252, -0.578252, -0.578252 },
		{ 0.708211, 0.000000, -0.708211 },
		{ 0.000000, -0.708211, -0.708211 },
		{ -0.708211, 0.000000, -0.708211 },
		{ 0.000000, 0.708211, -0.708211 },
		{ 0.000000, 0.000000, -1.001561 }
	};

	const vertexTexCoords t[] = {
		{ 0.500000, 0.500000 },
		{ 1.000000, 0.000000 },
		{ 0.500000, 0.000000 },
		{ 0.000000, 0.000000 },
		{ 0.000000, 0.500000 },
		{ 0.000000, 1.000000 },
		{ 1.000000, 1.000000 },
		{ 0.500000, 1.000000 },
		{ 1.000000, 0.500000 }
	};

	const vertexPosition trianglePositions[] =
	{
		p[8], p[2], p[7],
		p[8], p[0], p[4],
		p[1], p[8], p[4],
		p[3], p[8], p[5],
		p[8], p[6], p[2],
		p[8], p[7], p[0],
		p[1], p[5], p[8],
		p[3], p[6], p[8]
	};

	const vertexTexCoords triangleUVs[] =
	{
		t[0], t[1], t[2],
		t[0], t[3], t[4],
		t[5], t[0], t[4],
		t[6], t[0], t[7],
		t[0], t[8], t[1],
		t[0], t[2], t[3],
		t[5], t[7], t[0],
		t[6], t[8], t[0]
	};
	int i = sizeof(trianglePositions);
	glGenBuffers(1, &model->vertexPositionbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexPositionbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(trianglePositions), trianglePositions, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(triangleUVs), triangleUVs, GL_STATIC_DRAW);

	model->indices = 8 * 3;
}

void Render_DrawModel(Model model)
{

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexPositionbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Draw
	glDrawArrays(GL_TRIANGLES, 0, model.indices);
	PRINT_GL_ERRORS;
}

GLuint Render_CreateTexture(int textureSlot, int width, int height, GLenum format, unsigned char* pixels)
{
	GLuint texture;

	glActiveTexture(GL_TEXTURE0 + textureSlot);
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	// Pros to Linear: Looks WAY WAY WAY WAY BETTER
	// Cons to Linear: Minor border seams on panoramas are visible during loading, but largely invisible at max depth
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, format, GL_UNSIGNED_BYTE, pixels);
	PRINT_GL_ERRORS;

	return texture;
}

void Render_CreateTexture(Texture* out_texture, unsigned int textureSlot, unsigned int width, unsigned int height, GLenum format, unsigned char *pixels)
{
	out_texture->id = Render_CreateTexture(textureSlot, width, height, format, pixels);
	out_texture->width = width;
	out_texture->height = height;
}