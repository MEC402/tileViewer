#include "stdafx.h"
#include "PanoInfo.h"

#include <string>
#include <vector>
#include <algorithm>
#include <windows.h>
#include <thread>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int ImageHandler::m_maxWidth[6];
int ImageHandler::m_maxHeight[6];
int ImageHandler::m_faceWidth[6];
int ImageHandler::m_faceHeight[6];
GLuint ImageHandler::m_textures[6];
const char *ImageHandler::m_txUniforms[6] = { "TxFront", "TxBack", "TxRight", "TxLeft", "TxTop", "TxBottom" };
const char ImageHandler::m_faceNames[6] = { 'f', 'b', 'r', 'l', 'u', 'd' };
int ImageHandler::m_tileDepth[6][8][8] = { { { 0 } } };
std::vector<PanoInfo> ImageHandler::m_panoList;

/* ---------------- Public Functions ---------------- */

void ImageHandler::InitTextureAtlas(GLuint program) 
{
	glGenTextures(6, m_textures);

	// hardcoded magic
	int maxDepth = 3;

	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, maxDepth, program);
		LoadFaceImage(i, 0);
	}
}

void ImageHandler::InitPanoListFromOnlineFile(std::string url)
{
	ImageData jsonFile;
	downloadFile(&jsonFile, url);
	if (jsonFile.data) {
		std::string fileAsString(jsonFile.data, jsonFile.data + jsonFile.dataSize);
		// Base URL is the substring before the last backslash or forward slash
		size_t lastSlashPosition = url.find_last_of("/\\");
		std::string baseURL = url.substr(0, lastSlashPosition);
		m_panoList = parsePanoInfoFile(fileAsString, baseURL);
	}
}

void ImageHandler::LoadImageData(ImageData *image)
{
	//if (image.depth < m_tileDepth[image.face][image.h_offset][image.w_offset]) {
	//	return;
	//}
	//m_tileDepth[image.face][image.h_offset][image.w_offset] = image.depth;

	int width, height, nrChannels;
	unsigned char *d = stbi_load_from_memory((stbi_uc*)image->data, image->dataSize,
		&width, &height, &nrChannels, 0);

	if (d) {
		glActiveTexture(image->activeTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, image->w_offset * width, image->h_offset * height,
			width, height, GL_RGB, GL_UNSIGNED_BYTE, d);

		GLenum errCode;
		if ((errCode = glGetError()) != GL_NO_ERROR) {
			const GLubyte *errString = gluErrorString(errCode);
			printf("OPENGL ERROR: %s\n", errString);
		}
	}
	else {
		fprintf(stderr, "Error loading image file!\n");
	}
	stbi_image_free(d);
	delete image;
}

void ImageHandler::LoadQuadImage(int face, int row, int col, int depth)
{
	// Invert rows because I structured this like a complete maniac and now I don't know how to undo the evil that is Aku
	row = 7 - row;

	// Magic number so we can set our xOffset correctly in the texture atlas
	int numQuadsToChange = 8 / (int)pow(2, depth);

	// Calculate the relative quad based on the depth 
	// e.g. If we're at level 1, on row 7, 7%2 -> 1, which is the correct texture for that given quad
	//int depthQuadRow = (int)floor(row % (int)pow(2, depth));
	int depthQuadRow = row / numQuadsToChange;

	// Actually calculate our xOffset for the texture atlas
	int depthQuadCol = col / numQuadsToChange;

	// *Theoretically* this should prevent us from overwriting a higher res texture with a lower res one
	if (m_tileDepth[face][depthQuadRow][depthQuadCol] >= depth) {
		return;
	}
	m_tileDepth[face][depthQuadRow][depthQuadCol] = depth;

	const int bufferSize = 128;
	char buf[bufferSize];
	sprintf_s(buf, bufferSize, m_panoList[0].leftAddress.c_str(), depth + 1, m_faceNames[face], depthQuadRow, depthQuadCol);

	ImageData *imageFile = new ImageData{ 0 };
	downloadFile(imageFile, buf);
	imageFile->w_offset = depthQuadCol;
	imageFile->h_offset = depthQuadRow;
	imageFile->activeTexture = GL_TEXTURE0 + face;
	imageFile->face = face;
	ImageQueue::Enqueue(imageFile);
}

void ImageHandler::LoadFaceImage(int face, int depth)
{
	const char facename = m_faceNames[face];
	int activeTexture = GL_TEXTURE0 + face;

	fprintf(stderr, "Loading tile data for face: %c At depth level: %d\n", facename, depth+1);
	
	// This seems to work pretty nicely
	int maxDepth = (int)pow(2, depth); // Get the 2^n maximal depth to search for
	std::vector<std::string> urls;

	for (int i = 0; i < maxDepth; i++) {

		int bufferSize = 256;
		char buf[256];
		for (int j = 0; j < maxDepth; j++) {
			sprintf_s(buf, m_panoList[0].leftAddress.c_str(), depth + 1, m_faceNames[face], i, j);
			urls.push_back(buf);
		}
	}

	//std::vector<ImageData> imageFiles(urls.size());
	ImageData **imageFiles = new ImageData*[urls.size()];
	for (int i = 0; i < maxDepth * maxDepth; i++) {
		imageFiles[i] = new ImageData{ 0 };
	}
	//downloadMultipleFiles(imageFiles, urls.data(), urls.size());
	//std::thread t(downloadMultipleFiles, imageFiles, urls.data(), urls.size());
	//t.detach();
	//t.join();
	Threads::DefaultThreadPool::submitJob(downloadMultipleFiles, imageFiles, urls.data(), urls.size());


	int i = 0;
	while(i < (maxDepth * maxDepth)) {
		if (imageFiles[i]->complete) {
			imageFiles[i]->activeTexture = activeTexture;
			imageFiles[i]->face = face;
			ImageQueue::Enqueue(imageFiles[i]);

			// Reset complete flag to false so we aren't double-counting
			imageFiles[i]->complete = false;
			i++;
		}
	}
	m_faceWidth[face] = 512 * maxDepth;
	m_faceHeight[face] = 512 * maxDepth;
	delete[]imageFiles;
}

float ImageHandler::TxScalingX(int face)
{
	return (float)m_faceWidth[face] / (float)m_maxWidth[face];
}

float ImageHandler::TxScalingY(int face)
{
	return (float)m_faceHeight[face] / (float)m_maxHeight[face];
}


// For use after doing a hot-reload on shaders
void ImageHandler::RebindTextures(GLuint program)
{
	for (int i = 0; i < 6; i++) {
		GLuint TxUniform = glGetUniformLocation(program, m_txUniforms[i]);
		if (TxUniform == -1) {
			fprintf(stderr, "Error getting %s uniform\n", m_txUniforms[i]);
		}
		else {
			glUniform1i(TxUniform, i);
		}
	}
}

/* ---------------- Private Functions ---------------- */

//void ImageHandler::threadedImageLoad(const char *path, int depth, const char *facename, int i, int j, imageData *data)
//{
//	// Directory structure is: path\\depth level\\facename\\row\\column
//	// Depth level is 1 indexed instead of 0 indexed (but row/col are 0 indexed?), so we're formatting with depth+1 
//	char buf[60];
//	sprintf_s(buf, 60, "%s\\%d\\%s\\%d\\%d.jpg",
//		path, depth + 1, facename, i, j);
//	int width, height, nrChannels;
//	unsigned char *d = stbi_load(buf, &width, &height, &nrChannels, 0);
//	if (d) {
//		*data = { d, width, height, width * j, height * i };
//	}
//	else {
//		fprintf(stderr, "Failed to load image\n");
//	}
//	// Don't call stbi_free() here, call it back in LoadImageFromPath after we try to dump images into the GPU
//}

void ImageHandler::initFaceAtlas(int face, int depth, GLuint program)
{
	//int unused;
	////TODO: Need to look at the deepest level *for the given face*, not just hardcoded
	//stbi_info("f_0.jpg", &m_maxWidth[face], &m_maxHeight[face], &unused);
	//
	//m_maxWidth[face] *= pow(2, depth);
	//m_maxHeight[face] *= pow(2, depth);
	m_maxWidth[face] = 512 * (int)pow(2, depth);
	m_maxHeight[face] = 512 * (int)pow(2, depth);

	const char *uniform = m_txUniforms[face];
	glActiveTexture(GL_TEXTURE0 + face);

	glBindTexture(GL_TEXTURE_2D, m_textures[face]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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

// Gnarly WIN32 API calls to traverse a given directory and find out max resolution depth
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
	for (unsigned int i = 0; i < output.size(); i++) {
		if (output[i] > depth)
			depth = output[i];
	}

	// Reduce depth by 1, since folders are listed 1/2/3/4 instead of 0/1/2/3
	return depth-1;
}