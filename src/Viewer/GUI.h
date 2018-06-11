#pragma once
#include "InternetDownload.h"
#include "stb_image.h"
#include <GL\glew.h>
#include "PanoInfo.h"
#include "VR.h"

struct Model
{
	GLuint vertexPositionbuffer;
	GLuint vertexUVBuffer;
	unsigned int indices;
};

struct GraphicalInterface
{
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	Model quad;
};

void createModelFromQuad(Model* model);
void renderModel(Model model, GLuint tex, OVR::Matrix4f mvp);

void createGUI(GraphicalInterface* gui, std::vector<PanoInfo> panoList);
void displayGUI(GraphicalInterface gui, OVR::Matrix4f viewProjection, float panoSelection);