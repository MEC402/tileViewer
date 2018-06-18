#pragma once
#include "InternetDownload.h"
#include <GL\glew.h>
#include "PanoInfo.h"
#include "Shader.h"
#include "Render.h"
#include "VR.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"


class GraphicalMenu
{
public:
	void create(std::vector<PanoInfo> panoList);
	void display(glm::quat headsetRotation, glm::mat4x4 viewProjection, float radius, float panoSelection);

private:
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	Model quad;
	Shader shader;
};