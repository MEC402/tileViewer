#include "stdafx.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int ImageHandler::m_maxWidth[6];
int ImageHandler::m_maxHeight[6];
int ImageHandler::m_faceWidth[6];
int ImageHandler::m_faceHeight[6];
GLuint ImageHandler::m_textures[6];

void ImageHandler::initFaceAtlas(int face, GLuint program, const char *path)
{
	int unused;
	//TODO: Need to look at the deepest level *for this face*, not just hardcoded
	stbi_info("f_0.jpg", &m_maxWidth[face], &m_maxHeight[face], &unused);
	const char *uniform = "";
	switch (face) {
	case 0:
		uniform = "TxFront";
		glActiveTexture(GL_TEXTURE0);
		break;
	case 1:
		uniform = "TxBack";
		glActiveTexture(GL_TEXTURE1);
		break;
	case 2:
		uniform = "TxRight";
		glActiveTexture(GL_TEXTURE2);
		break;
	case 3:
		uniform = "TxLeft";
		glActiveTexture(GL_TEXTURE3);
		break;
	case 4:
		uniform = "TxTop";
		glActiveTexture(GL_TEXTURE4);
		break;
	case 5:
		uniform = "TxBottom";
		glActiveTexture(GL_TEXTURE5);
		break;
	}
	
	glBindTexture(GL_TEXTURE_2D, m_textures[face]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_maxWidth[face], m_maxHeight[face], 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	LoadImageFromPath(path, face);
	GLuint TxUniform = glGetUniformLocation(program, uniform);
	if (TxUniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", uniform);
	}
	else {
		glUniform1i(TxUniform, face);
	}
}

void ImageHandler::InitTextureAtlas(GLuint program) 
{
	glGenTextures(6, m_textures);
	// TODO: This needs to be not hardcoded
	initFaceAtlas(0, program, "f_0.jpg");
	initFaceAtlas(1, program, "b_0.jpg");
	initFaceAtlas(2, program, "r_0.jpg");
	initFaceAtlas(3, program, "l_0.jpg");
	initFaceAtlas(4, program, "u_0.jpg");
	initFaceAtlas(5, program, "d_0.jpg");
}

void ImageHandler::LoadImageFromPath(const char *path, int face)
{
	int width, height, nrChannels;
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data) {
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
	}
	else {
		fprintf(stderr, "Failed to load image");
	}
	stbi_image_free(data);
	m_faceWidth[face] = width;
	m_faceHeight[face] = height;
}

float ImageHandler::TxScalingX(int face)
{
	return (float)m_faceWidth[face] / (float)m_maxWidth[face];
}

float ImageHandler::TxScalingY(int face)
{
	return (float)m_faceHeight[face] / (float)m_maxHeight[face];
}