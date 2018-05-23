// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <thread>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Oculus.h"
#include "Shader.h"
#include "CubeData.h"
#include "PanoLoading.h"

using namespace glm;

#define PRINT_GL_ERRORS \
{\
	GLenum errCode;\
	if ((errCode = glGetError()) != GL_NO_ERROR) {\
		const GLubyte *errString = gluErrorString(errCode);\
		printf("OpenGL error on line %d in %s: %s\n", __LINE__, __FUNCTION__, errString);\
	}\
}\

struct Model
{
	GLuint vertexPositionbuffer;
	GLuint vertexUVBuffer;
	unsigned int indices;
};

struct Texture
{
	GLuint id;
	int width;
	int height;
};

struct CubeTextures
{
	enum FaceIndex {
		front, back, left, right, up, down
	};

	struct Eye {
		Texture faces[6];
	};

	Eye eye[2];
	Texture tempTextureBuffer;
	unsigned int currentResolution;
	GLuint blitSourceFramebuffer;
	GLuint blitDestFramebuffer;
};

void createTexture(Texture* out_texture, unsigned int width, unsigned int height, char* pixels)
{
	out_texture->width = width;
	out_texture->height = height;
	glGenTextures(1, &out_texture->id);
	glBindTexture(GL_TEXTURE_2D, out_texture->id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void createCubeTextures(CubeTextures* cube, unsigned int maxResolution, unsigned int tileResolution)
{
	*cube = { 0 };
	for (unsigned int i = 0; i < 6; ++i)
	{
		createTexture(&cube->eye[0].faces[i], maxResolution, maxResolution, 0);
		createTexture(&cube->eye[1].faces[i], maxResolution, maxResolution, 0);
	}

	glGenFramebuffers(1, &cube->blitSourceFramebuffer);
	glGenFramebuffers(1, &cube->blitDestFramebuffer);
	createTexture(&cube->tempTextureBuffer, tileResolution, tileResolution, 0);
}

void createModelFromQuad(Model* model)
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

	PRINT_GL_ERRORS;
}

GLFWwindow* setupWindowAndOpenGL()
{
	GLFWwindow* window = 0;
	// Initialise GLFW
	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		getchar();
		return 0;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	PRINT_GL_ERRORS;

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "Tutorial 04 - Colored Cube", NULL, NULL);
	if (window == NULL) {
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		getchar();
		glfwTerminate();
		return 0;
	}
	glfwMakeContextCurrent(window);

	PRINT_GL_ERRORS;

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		getchar();
		glfwTerminate();
		return 0;
	}

	PRINT_GL_ERRORS;

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


	PRINT_GL_ERRORS;

	return window;
}

void renderModel(Model model, Texture tex, GLuint programID, OVR::Matrix4f mvp)
{
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_TRUE, (float*)&mvp);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex.id);
	GLuint sampler = glGetUniformLocation(programID, "image");
	glUniform1i(sampler, 0);
	//glBindSampler(tex.id, 0);

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexPositionbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Darw
	glDrawArrays(GL_TRIANGLES, 0, model.indices);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);

	PRINT_GL_ERRORS;
}

void textureDownloadThread(std::vector<std::string> urlList, DownloadedImage* images)
{
	std::vector<DownloadedFile*> files(urlList.size());
	for (unsigned int i = 0; i < urlList.size(); ++i) {
		files[i] = &images[i].file;
	}

	downloadMultipleFiles(files.data(), urlList.data(), urlList.size());
}

void textureDecompressThread(DownloadedImage* images, unsigned int imageCount)
{
	bool allImagesDecompressed = false;
	while (allImagesDecompressed == false)
	{
		allImagesDecompressed = true;
		for (unsigned int i = 0; i < imageCount; ++i)
		{
			if (images[i].file.complete) {
				int nrChannels;
				images[i].decompressedPixels = stbi_load_from_memory((stbi_uc*)images[i].file.data, images[i].file.dataSize, &images[i].width, &images[i].height, &nrChannels, 0);
			}
			else {
				allImagesDecompressed = false;
			}
		}
	}
}

void startTextureDownloadingThread(PanoInfo pano, unsigned int loadResolutionLevel, std::vector<DownloadedImage>* images)
{
	const unsigned int bufferSize = 256;
	char buf[bufferSize];
	unsigned int rows = (unsigned int)pow(2, loadResolutionLevel-1);
	unsigned int columns = rows;
	images->resize(0);
	unsigned int imageIndex = 0;

	std::vector<std::string> urlList;

	for (unsigned int eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
	{
		for (unsigned int faceIndex = 0; faceIndex < 6; ++faceIndex)
		{
			char face = 'f';
			switch (faceIndex) {
			case CubeTextures::front: face = 'f'; break;
			case CubeTextures::back: face = 'b'; break;
			case CubeTextures::left: face = 'l'; break;
			case CubeTextures::right: face = 'r'; break;
			case CubeTextures::up: face = 'u'; break;
			case CubeTextures::down: face = 'd'; break;
			}
			std::string& eyeAddress = (eyeIndex == 0) ? pano.leftAddress : pano.rightAddress;

			for (unsigned int row = 0; row < rows; ++row)
			{
				for (unsigned int column = 0; column < columns; ++column)
				{
					// Goes resolution level, face, row, column
					sprintf_s(buf, eyeAddress.c_str(), loadResolutionLevel, face, row, column);
					std::string url = buf;
					urlList.push_back(url);

					DownloadedImage image = { 0 };
					image.eyeIndex = eyeIndex;
					image.faceIndex = faceIndex;
					image.row = row;
					image.column = column;
					images->push_back(image);
				}
			}
		}
	}

	// TODO: join the threads instead of detaching
	std::thread downloader(textureDownloadThread, urlList, images->data());
	downloader.detach();
	std::thread decompresser(textureDecompressThread, images->data(), images->size());
	decompresser.detach();
}

void readImages(CubeTextures* cube, std::vector<DownloadedImage>& images, unsigned int loadResolutionLevel, unsigned int maxImagesToLoad, bool* out_allImagesRead)
{
	const unsigned int rows = (unsigned int)pow(2, loadResolutionLevel-1);
	const unsigned int columns = rows;
	unsigned int loadedImages = 0;
	
	*out_allImagesRead = true;

	for (unsigned int i = 0; i < images.size(); ++i)
	{
		DownloadedImage& image = images[i];
		if (!image.file.complete || image.file.data) {
			*out_allImagesRead = false;
		}
		if (loadedImages < maxImagesToLoad && image.decompressedPixels)
		{
			// GPU scaling
			// Put the texture in a temporary buffer for blitting
			glBindTexture(GL_TEXTURE_2D, cube->tempTextureBuffer.id);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.width, image.height, GL_RGB, GL_UNSIGNED_BYTE, image.decompressedPixels);

			// Stretch the loaded image to the full cube face texture
			glBindFramebuffer(GL_FRAMEBUFFER, cube->blitSourceFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cube->tempTextureBuffer.id, 0);
			glBindFramebuffer(GL_FRAMEBUFFER, cube->blitDestFramebuffer);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cube->eye[image.eyeIndex].faces[image.faceIndex].id, 0);

			int destWidth = cube->eye[image.eyeIndex].faces[image.faceIndex].width / rows;
			int destHeight = cube->eye[image.eyeIndex].faces[image.faceIndex].height / columns;
			int destX = image.column*destWidth;
			int destY = image.row*destHeight;
			glBlitNamedFramebuffer(cube->blitSourceFramebuffer, cube->blitDestFramebuffer,
				0, 0, cube->tempTextureBuffer.width, cube->tempTextureBuffer.height, // Source
				destX, destY, destX + destWidth, destY + destHeight, // Destination
				GL_COLOR_BUFFER_BIT, GL_NEAREST);

			// Free memory
			stbi_image_free(image.decompressedPixels);
			image.decompressedPixels = 0;
			free(image.file.data);
			image.file.data = 0;
			image.file.dataSize = 0;
			
			++loadedImages;
		}
	}
}

int main(int argc, char * argv[])
{
	GLFWwindow* window = setupWindowAndOpenGL();
	if (!window) {
		return -1;
	}

	std::vector<PanoInfo> panoList = InitPanoListFromOnlineFile(argv[1]);

	const unsigned int maxResolution = 4096;
	const unsigned int tileResolution = 512;
	const unsigned int maxResolutionLevel = 4;
	unsigned int loadResolutionLevel = 1;
	CubeTextures cubeTextures = { 0 };
	createCubeTextures(&cubeTextures, maxResolution, tileResolution);
	std::vector<DownloadedImage> downloadImageList;
	bool startLoading = false;

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	PRINT_GL_ERRORS;

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "ColorFragmentShader.fragmentshader");

	Model quad;
	createModelFromQuad(&quad);

	int viewportWidth, viewportHeight;
	glfwGetFramebufferSize(window, &viewportWidth, &viewportHeight);

	VRDevice oculus;
	createVRDevice(&oculus, viewportWidth, viewportHeight);

	glUseProgram(programID);

	PRINT_GL_ERRORS;

	// Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0)
	{
		const unsigned int maxImagesToLoadPerFrame = 1;
		bool allImagesRead;
		readImages(&cubeTextures, downloadImageList, loadResolutionLevel, maxImagesToLoadPerFrame, &allImagesRead);
		// If we processed all of the images, start loading the next resolution level
		if (startLoading && allImagesRead && loadResolutionLevel < maxResolutionLevel)
		{
			++loadResolutionLevel;
			startTextureDownloadingThread(panoList[0], loadResolutionLevel, &downloadImageList);
		}

		updateVRDevice(&oculus);
		VRControllerStates controllers = getVRControllerState(&oculus);

		float scale = .1f;
		OVR::Matrix4f rightHandCubeTransform = OVR::Matrix4f::Translation(controllers.right.position) * OVR::Matrix4f::Scaling(OVR::Vector3f(scale, scale, scale)) * OVR::Matrix4f(controllers.right.rotation);
		OVR::Matrix4f leftHandCubeTransform = OVR::Matrix4f::Translation(controllers.left.position) * OVR::Matrix4f::Scaling(OVR::Vector3f(scale, scale, scale)) * OVR::Matrix4f(controllers.left.rotation);
		
		// Render Scene to Eye Buffers
		for (int eye = 0; eye < 2; ++eye)
		{
			OVR::Matrix4f view = buildVRViewMatrix(&oculus, eye, 0, 0, 0) * OVR::Matrix4f::Translation(getVRHeadsetPosition(&oculus));
			OVR::Matrix4f projection = buildVRProjectionMatrix(&oculus, eye);

			bindEyeRenderSurface(&oculus, eye);

			ovrInputState inputState;
			ovr_GetInputState(oculus.session, ovrControllerType_Touch, &inputState);

			// Clear the screen
			float red = (inputState.Thumbstick[1].x + 1) / 2;
			float green = inputState.IndexTrigger[1];
			float blue = (inputState.Thumbstick[1].y + 1) / 2;
			glClearColor(red, green, blue, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glDisable(GL_CULL_FACE);

			// Render cube
			float pi = 3.14159f;
			OVR::Matrix4f forward = OVR::Matrix4f::Translation(0, 0, 1);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::front], programID, projection * view * forward);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::right], programID, projection * view * OVR::Matrix4f::RotationY(pi/2) * forward);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::left], programID, projection * view * OVR::Matrix4f::RotationY(-pi / 2) * forward);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::back], programID, projection * view * OVR::Matrix4f::RotationY(pi) * forward);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::up], programID, projection * view * OVR::Matrix4f::RotationX(-pi / 2) * forward);
			renderModel(quad, cubeTextures.eye[eye].faces[CubeTextures::down], programID, projection * view * OVR::Matrix4f::RotationX(pi / 2) * forward);

			commitEyeRenderSurface(&oculus, eye);
		}

		finishVRFrame(&oculus);
		blitHeadsetView(&oculus, 0);

		// Swap buffers
		glfwSwapBuffers(window);

		// Process window messages
		glfwPollEvents();
		// Handle window resize
		int newViewportWidth, newViewportHeight;
		glfwGetFramebufferSize(window, &newViewportWidth, &newViewportHeight);
		if (newViewportWidth != viewportWidth || newViewportHeight != viewportHeight) {
			resizeMirrorTexture(&oculus, newViewportWidth, newViewportHeight);
			viewportWidth = newViewportWidth;
			viewportHeight = newViewportHeight;
		}

		// Keyboard events
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			if (!startLoading) {
				startTextureDownloadingThread(panoList[0], loadResolutionLevel, &downloadImageList);
			}
			startLoading = true;
		}
		/*if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			loadResolutionLevel = 0;
		}*/

		PRINT_GL_ERRORS;
	}

	// Cleanup VBO and shader
	/*glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &colorbuffer);
	glDeleteProgram(programID);
	glDeleteVertexArrays(1, &VertexArrayID); */

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	destroyVRDevice(&oculus);

	return 0;
}

