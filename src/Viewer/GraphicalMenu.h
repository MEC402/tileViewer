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

#define THUMB_TX_SLOT 13

class GraphicalMenu
{
public:
	void create(std::vector<PanoInfo> panoList);
	void display(glm::quat headsetRotation, glm::mat4x4 viewProjection, float radius, float panoSelection, bool tilt);
	void display(glm::quat cameraRotation, glm::mat4x4 viewProjection, float panoSelection);


private:
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	Model quad;
	Shader shader;
};

#endif // _GRAPHICALMENU_H