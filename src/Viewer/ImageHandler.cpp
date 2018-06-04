//#include "stdafx.h"

#include "ImageHandler.h"
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <mutex>

int ImageHandler::m_currentPano = 0;

std::mutex ImageHandler::m_;
GLuint ImageHandler::m_textures[2][6];
GLuint ImageHandler::m_pbos[2][6];
const char *ImageHandler::m_txUniforms[6] = { "TxFront", "TxBack", "TxRight", "TxLeft", "TxTop", "TxBottom" };
const char ImageHandler::m_faceNames[6] = { 'f', 'b', 'r', 'l', 'u', 'd' };
std::vector<PanoInfo> ImageHandler::m_panoList;

// I'm_ not convinced this is necessary here
int ImageHandler::m_tileDepth[6][8][8] = { { { 0 } } };

/* ---------------- Public Functions ---------------- */

void ImageHandler::InitTextureAtlas(GLuint program, bool stereo) 
{
	// 6 for each eye
	glGenTextures(6, m_textures[0]);

	if (stereo)
		glGenTextures(6, m_textures[1]);

	// hardcoded magic
	int maxDepth = 3;

	// Init the texture atlas for each face, then go ahead and load in the first image
	// May behoove us to factor out LoadFaceImage from this loop and call it separately for
	// better flow control
	for (int i = 0; i < 6; i++) {
		initFaceAtlas(i, maxDepth, 0, program);
		//LoadQuadImage(i, 0, 0, 0, 0);
		if (stereo) {
			initFaceAtlas(i, maxDepth, 1, program);
			//LoadQuadImage(i, 0, 0, 0, 1);
		}
	}
}

void ImageHandler::InitPanoListFromOnlineFile(std::string url)
{
	ImageData jsonFile;
	downloadFile(&jsonFile, url);
	try {
		if (jsonFile.data) {
			std::string fileAsString(jsonFile.data, jsonFile.data + jsonFile.dataSize);
			//unsigned char** datacopy = jsonFile.data.get();
			//std::string fileAsString(*datacopy, *datacopy + jsonFile.dataSize);
			// Base URL is the substring before the last backslash or forward slash
			size_t lastSlashPosition = url.find_last_of("/\\");
			std::string baseURL = url.substr(0, lastSlashPosition);
			m_panoList = parsePanoInfoFile(fileAsString, baseURL);
		}
	}
	catch (const std::exception &exc) {
		fprintf(stderr, "%s\n", exc.what());
	}
}

void ImageHandler::LoadImageData(ImageData *image)
{
	GLenum errCode;
	// TODO: Need to include a given images width/height so we're not hardcoding 512x512
	if (image->data) {
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[image->eye][image->face]);
		if ((errCode = glGetError()) != GL_NO_ERROR) {
			printf("OPENGL ERROR BINDBUFFER: %s\n", gluErrorString(errCode));
		}

		int* dst = (int*)glMapBuffer(GL_PIXEL_UNPACK_BUFFER, GL_WRITE_ONLY);
		if (dst) {
			std::memcpy(dst, image->data, 512 * 512 * 3);
		}

		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

		glActiveTexture(image->activeTexture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, image->w_offset, image->h_offset, 512, 512, 
			GL_RGB, GL_UNSIGNED_BYTE, nullptr);

		if ((errCode = glGetError()) != GL_NO_ERROR) {
			printf("OPENGL ERROR Loading Image: %s\n", gluErrorString(errCode));
		}
		
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	}
	else {
		fprintf(stderr, "Error loading image file! No pixel data found\n");
	}
	image->Free();
}

void ImageHandler::LoadQuadImage(int face, int row, int col, int depth, int eye)
{
	// ST coordinates are inverted along the Y axis, flip our row value
	row = 7 - row;
	//col = 7 - col;

	// Same math used in CubePoints::QuadNextDepth to calculate which image to load
	int numQuadsToChange = 8 / (int)pow(2, depth);
	int depthQuadRow = row / numQuadsToChange;
	int depthQuadCol = col / numQuadsToChange;

	// Set our URI string to load the image
	const int bufferSize = 128;
	char buf[bufferSize];
	if (eye == 0) {
		sprintf_s(buf, bufferSize, m_panoList[m_currentPano].leftAddress.c_str(), depth + 1, m_faceNames[face], depthQuadRow, depthQuadCol);
	}
	else {
		sprintf_s(buf, bufferSize, m_panoList[m_currentPano].rightAddress.c_str(), depth + 1, m_faceNames[face], depthQuadRow, depthQuadCol);
	}

	// Load the image, set ImageData values for later use
	ImageData *imageFile = new ImageData{ 0 };
	downloadFile(imageFile, buf);

	// stbi_load_from_memory mallocs data using the first param
	// Store that malloc to a new pointer, free our old one, then reassign
	// Otherwise we get a big memory leak
	int width, height, nrChannels;
	unsigned char* d = (unsigned char*)stbi_load_from_memory((stbi_uc*)imageFile->data, imageFile->dataSize, &width, &height, &nrChannels, 0);

	free(imageFile->data);

	// Populate all our fields
	imageFile->data = d;
	imageFile->w_offset = depthQuadCol;
	imageFile->h_offset = depthQuadRow;
	imageFile->activeTexture = GL_TEXTURE0 + face + (6 * eye);
	imageFile->face = face;
	imageFile->eye = eye;
	imageFile->w_offset *= width;
	imageFile->h_offset *= height;

	// Throw it into our queue
	ImageQueue::Enqueue(imageFile);
	// Do NOT delete the pointer here, we free that memory after loading it into the texture atlas
}

void ImageHandler::LoadFaceImage(int face, int depth, int eye)
{
	// Basically the same as LoadQuadImage
	const char facename = m_faceNames[face];
	int activeTexture = GL_TEXTURE0 + face + (6 * eye);
	
	// This seems to work pretty nicely
	int maxDepth = (int)pow(2, depth); // Get the 2^n maximal depth to search for
	std::vector<std::string> urls;

	// Populate our URL vectors in reverse, this helps make loading in images appear smoother (we don't overwrite lower-depth tiles until last)
	for (int i = maxDepth-1; i >= 0; i--) {
		int bufferSize = 256;
		char buf[256];
		for (int j = maxDepth-1; j >= 0; j--) {
			if (eye == 0) {
				sprintf_s(buf, m_panoList[m_currentPano].leftAddress.c_str(), depth + 1, m_faceNames[face], i, j);
			}
			else {
				sprintf_s(buf, m_panoList[m_currentPano].rightAddress.c_str(), depth + 1, m_faceNames[face], i, j);
			}
			urls.push_back(buf);
		}
	}

	// Initialize all our pointers on the heap ahead of time
	// Might be a way to do this on imageFiles declaration?
	ImageData **imageFiles = new ImageData*[urls.size()];
	for (int i = 0; i < maxDepth * maxDepth; i++) {
		imageFiles[i] = new ImageData{ 0 };
	}
	//downloadMultipleFiles(imageFiles, urls.data(), urls.size());
	std::thread t(downloadMultipleFiles, imageFiles, urls.data(), urls.size());
	t.detach();


	int i = 0;
	int width, height, nrChannels;
	while(i < (maxDepth * maxDepth)) {
		// Check the completion status of each image before trying to load it
		if (imageFiles[i]->complete) {
			// Same as LoadQuadImage, new pointers then free then reassign to avoid memory leaks
			unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)imageFiles[i]->data, imageFiles[i]->dataSize, &width, &height, &nrChannels, 0));

			free(imageFiles[i]->data);

			imageFiles[i]->data = d;
			imageFiles[i]->activeTexture = activeTexture;
			imageFiles[i]->face = face;
			imageFiles[i]->eye = eye;
			imageFiles[i]->w_offset *= width;
			imageFiles[i]->h_offset *= height;

			ImageQueue::Enqueue(imageFiles[i]);

			// Reset complete flag to false so we aren't double-counting (This is only set to true in InternetDownloader)
			imageFiles[i]->complete = false;
			i++;
		}
	}
	// No longer need the pointer for our array of ImageData pointers, they're all in the ImageQueue now
	delete[]imageFiles;
}

// For use after doing a hot-reload on shaders (Or switching between two sets of Texture Atlases)
void ImageHandler::RebindTextures(GLuint program, int eye)
{
	for (int i = 0; i < 6; i++) {
		GLuint TxUniform = glGetUniformLocation(program, m_txUniforms[i]);
		if (TxUniform == -1) {
			fprintf(stderr, "Error getting %s uniform\n", m_txUniforms[i]);
		}
		else {
			glUniform1i(TxUniform, i + (6 * eye));
		}
	}
}

/* ---------------- Private Functions ---------------- */


void ImageHandler::initFaceAtlas(int face, int depth, int eye, GLuint program)
{
	// TODO: Probably shouldn't hardcode image resolution like this
	int maxWidth = 512 * (int)pow(2, depth);
	int maxHeight = 512 * (int)pow(2, depth);

	const char *uniform = m_txUniforms[face];
	glActiveTexture(GL_TEXTURE0 + face + (eye * 6));

	glBindTexture(GL_TEXTURE_2D, m_textures[eye][face]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Use nearest to prevent black borders from fragment shader interpolation
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, maxWidth, maxHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	GLuint TxUniform = glGetUniformLocation(program, uniform);
	if (TxUniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", uniform);
	}
	else {
		glUniform1i(TxUniform, face);
	}

	glGenBuffers(1, &m_pbos[eye][face]);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, m_pbos[eye][face]);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 512 * 512 * 3, 0, GL_STREAM_DRAW);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
}

#ifdef _USE_WIN_H
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
#endif