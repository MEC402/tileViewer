#ifndef _GRAPHICALMENU_H
#define _GRAPHICALMENU_H

#include <GL\glew.h>
#include "InternetDownload.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "PanoInfo.h"
#include "Render.h"
#include "Shader.h"
#include "stb_image.h"
#include "VR.h"

class GraphicalMenu
{
public:
	void Create(std::vector<PanoInfo> panoList);
	void Display(glm::quat headsetRotation, glm::mat4x4 viewProjection, float radius, float panoSelection, bool tilt);
	void Display(glm::quat cameraRotation, glm::mat4x4 viewProjection, float panoSelection);

	GraphicalMenu() : thumbnailCount(0), thumbnails(0) {}


private:
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	Model quad;
	Shader shader;
};

#endif // _GRAPHICALMENU_H