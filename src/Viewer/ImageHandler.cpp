#include "stdafx.h"
#include <string>
#include <vector>
#include <windows.h>
#include <thread>

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

	// TODO: This needs to be not hardcoded
	const char *path = "C:\\Users\\W8\\Desktop\\ThreeCroses4Left\\left";
	int maxDepth = maxResDepth(path);
	fprintf(stderr, "Maximum depth level found: %d\n", maxDepth);

	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, maxDepth, program);
		LoadImageFromPath(path, i, 0);
	}

	//std::thread threads[6];
	////TODO Parameterize depth levels
	//for (int i = 0; i < 6; i++) {
	//	initFaceAtlas(i, maxDepth, program);
	//	threads[i] = std::thread(LoadImageFromPath, path, i, maxDepth);
	//}
	//for (int i = 0; i < 6; i++) {
	//	threads[i].join();
	//}
	fprintf(stderr, "Done loading images\n");
}

void ImageHandler::LoadImageFromPath(const char *path, int face, int depth)
{
	const char *facename = "";
	int activeTexture;
	switch (face) {
	case 0:
		facename = "f";
		activeTexture = GL_TEXTURE0;
		break;
	case 1:
		facename = "b";
		activeTexture = GL_TEXTURE1;
		break;
	case 2:
		facename = "r";
		activeTexture = GL_TEXTURE2;
		break;
	case 3:
		facename = "l";
		activeTexture = GL_TEXTURE3;
		break;
	case 4:
		facename = "u";
		activeTexture = GL_TEXTURE4;
		break;
	case 5:
		facename = "d";
		activeTexture = GL_TEXTURE5;
		break;
	}

	fprintf(stderr, "Loading tile data for face: %s At depth level: %d\n", facename, depth+1);
	// This seems to work pretty nicely
	int w_offset = 0;
	int h_offset = 0;
	int maxDepth = pow(2, depth);
//	int width, height, nrChannels;
	std::vector<std::thread> threads(maxDepth);
	imageData *imgData = new imageData[maxDepth];
	//unsigned char *imgData = new unsigned char[maxDepth];
	stbi_set_flip_vertically_on_load(1);

	//std::vector<std::vector<unsigned char*>> imgData(maxDepth, std::vector<unsigned char*>(maxDepth));
	//for (int i = maxDepth - 1; i > -1; i--) {
	for (int i = 0; i < maxDepth; i++) {
		for (int j = 0; j < maxDepth; j++) {
			threads[j] = std::thread(threadedImageLoad, path, depth, facename, i, j, &imgData[j]);
		}
		for (int k = 0; k < maxDepth; k++) {
			threads[k].join();
		}
		for (int k = 0; k < maxDepth; k++) {
			imageData d = imgData[k];
			if (d.data) {
				glTexSubImage2D(GL_TEXTURE_2D, 0, d.w_offset, d.h_offset, d.width, d.height, GL_RGB, GL_UNSIGNED_BYTE, d.data);
			}
			else {
				fprintf(stderr, "Error loading image file!\n");
			}
			stbi_image_free(d.data);
		}
	}
	m_faceWidth[face] = 512 * maxDepth;
	m_faceHeight[face] = 512 * maxDepth;
	delete[] imgData;
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

void ImageHandler::threadedImageLoad(const char *path, int depth, const char *facename, int i, int j, imageData *data)
{
	char buf[60];
	sprintf_s(buf, 60, "%s\\%d\\%s\\%d\\%d.jpg",
		path, depth + 1, facename, i, j);
	int width, height, nrChannels;
	unsigned char *d = stbi_load(buf, &width, &height, &nrChannels, 0);
	if (d) {
		*data = { d, width, height, width * j, height * i };
	}
	else {
		fprintf(stderr, "Failed to load image\n");
	}
}

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
	WIN32_FIND_DATAA findfiledata;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::vector<int> output;
	char fullpath[MAX_PATH];
	GetFullPathNameA(path, MAX_PATH, fullpath, 0);
	std::string fp(fullpath);

	hFind = FindFirstFileA((fp + "\\*").c_str(), &findfiledata);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((findfiledata.dwFileAttributes | FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY
				&& (findfiledata.cFileName[0] != '.'))
			{
				fprintf(stderr, "Found: %s\n", findfiledata.cFileName);
				output.push_back(atoi(findfiledata.cFileName));
			}
		} while (FindNextFileA(hFind, &findfiledata) != 0);
	}
	int depth = 0;
	for (int i = 0; i < output.size(); i++) {
		if (output[i] > depth)
			depth = output[i];
	}

	return depth-1;
}