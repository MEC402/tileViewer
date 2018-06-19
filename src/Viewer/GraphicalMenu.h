#ifndef _GRAPHICALMENU_H
#define _GRAPHICALMENU_H

#include <GL\glew.h>
#include <chrono>
#include "InternetDownload.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "PanoInfo.h"
#include "Render.h"
#include "Shader.h"
#include "stb_image.h"
#include "VR.h"

// The first 12 active textures are used by CubeMap faces
// This is set as a macro for ease of refactoring later, should we want to use more slots later
#define THUMB_TX_SLOT 13 

class GraphicalMenu
{
public:
	void Create(std::vector<PanoInfo> panoList);
	void Display(glm::quat headsetRotation, glm::mat4x4 viewProjection, float radius, float panoSelection, bool tilt);
	void Display(glm::quat cameraRotation, glm::mat4x4 viewProjection, float panoSelection);
	void ShowCube(glm::quat cameraRotation, glm::mat4x4 viewProjection, double time);

	void StartTimer(void);
	void ResetTimer(void);

private:
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	double tilt_timer{ 1.1 };

	Model quad;
	Model cube;
	Shader shader;
};

#endif // _GRAPHICALMENU_H