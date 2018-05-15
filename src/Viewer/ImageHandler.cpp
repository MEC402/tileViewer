#include "stdafx.h"
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int ImageHandler::m_maxWidth[6];
int ImageHandler::m_maxHeight[6];
int ImageHandler::m_faceWidth[6];
int ImageHandler::m_faceHeight[6];
GLuint ImageHandler::m_textures[6];

/* ---------------- Public Functions ---------------- */

void ImageHandler::InitTextureAtlas(GLuint program) 
{
	glGenTextures(6, m_textures);

	//maxResDepth("C:\\Users\\W8\\Desktop\\ThreeCroses4Left\\left");

	// TODO: This needs to be not hardcoded
	//initFaceAtlas(0, 3, program, "C:\\Users\\W8\\Desktop\\ThreeCroses4Left\\left\\4\\f\\0\\0.jpg");

	//TODO Parameterize depth levels
	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, 3, program);
		LoadImageFromPath("", i, 3);
	}
}

void ImageHandler::LoadImageFromPath(const char *path, int face, int depth)
{
	const char *facename = "";
	switch (face) {
	case 0:
		facename = "f";
		glActiveTexture(GL_TEXTURE0);
		break;
	case 1:
		facename = "b";
		glActiveTexture(GL_TEXTURE1);
		break;
	case 2:
		facename = "r";
		glActiveTexture(GL_TEXTURE2);
		break;
	case 3:
		facename = "l";
		glActiveTexture(GL_TEXTURE3);
		break;
	case 4:
		facename = "u";
		glActiveTexture(GL_TEXTURE4);
		break;
	case 5:
		facename = "d";
		glActiveTexture(GL_TEXTURE5);
		break;
	}

	// This seems to work pretty nicely
	int w_offset = 0;
	int h_offset = 0;
	int maxDepth = pow(2, depth);
	int width, height, nrChannels;
	for (int j = 0; j < maxDepth; j++) {
		for (int i = 0; i < maxDepth; i++) {
			char buf[60];
			sprintf_s(buf, 60, "C:\\Users\\W8\\Desktop\\ThreeCroses4Left\\left\\%d\\%s\\%d\\%d.jpg", 
				(depth+1), facename, j, i);
			unsigned char *d = stbi_load(buf, &width, &height, &nrChannels, 0);
			if (d) {
				glTexSubImage2D(GL_TEXTURE_2D, 0, w_offset, h_offset, width, height, GL_RGB, GL_UNSIGNED_BYTE, d);
			}
			else {
				fprintf(stderr, "Failed to load image");
			}
			stbi_image_free(d);
			w_offset += width;
		}
		h_offset += height;
		w_offset = 0;
	}
	
	m_faceWidth[face] = width * pow(2, depth);
	m_faceHeight[face] = height * pow(2, depth);
}

float ImageHandler::TxScalingX(int face)
{
	return (float)m_faceWidth[face] / (float)m_maxWidth[face];
}

float ImageHandler::TxScalingY(int face)
{
	return (float)m_faceHeight[face] / (float)m_maxHeight[face];
}

/* ---------------- Private Functions ---------------- */

void ImageHandler::initFaceAtlas(int face, int depth, GLuint program)
{
	int unused;
	//TODO: Need to look at the deepest level *for this face*, not just hardcoded
	stbi_info("f_0.jpg", &m_maxWidth[face], &m_maxHeight[face], &unused);
	
	m_maxWidth[face] *= pow(2, depth);
	m_maxHeight[face] *= pow(2, depth);

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
	GLuint TxUniform = glGetUniformLocation(program, uniform);
	if (TxUniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", uniform);
	}
	else {
		glUniform1i(TxUniform, face);
	}
}

int ImageHandler::maxResDepth(const char *path)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile((LPCWSTR)path, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE)
	{
		printf("FindFirstFile failed (%d)\n", GetLastError());
		return 1;
	}
	else
	{
		fprintf(stderr, "Found %s\n", FindFileData.cFileName);
		FindClose(hFind);
	}
	return 1;
}