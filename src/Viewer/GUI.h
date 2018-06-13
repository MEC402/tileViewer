#pragma once
#include "InternetDownload.h"
#include "stb_image.h"
#include <GL\glew.h>
#include "PanoInfo.h"
#include "Shader.h"
#include "VR.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

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
	Shader shader;
};

void createModelFromQuad(Model* model);
void renderModel(Model model, Shader& shader, GLuint tex, glm::mat4x4 mvp);

void createGUI(GraphicalInterface* gui, std::vector<PanoInfo> panoList);
void displayGUI(GraphicalInterface gui, glm::mat4x4 viewProjection, float panoSelection);