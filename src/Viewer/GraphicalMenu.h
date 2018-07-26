#ifndef _GRAPHICALMENU_H
#define _GRAPHICALMENU_H

#include <GL\glew.h>
#include <chrono>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "Image.h"
#include "PanoInfo.h"
#include "Render.h"
#include "Shader.h"

class GraphicalMenu
{
public:
	void Create(std::vector<PanoInfo> panoList);
	void LoadThumbnailTextures(void);
	void Display(glm::quat headsetRotation, glm::mat4x4 viewProjection, Shader* shader, float radius, float panoSelection, bool tilt);
	void Display(glm::quat cameraRotation, glm::mat4x4 viewProjection, Shader* shader, float panoSelection);
	void ShowCube(glm::quat cameraRotation, glm::mat4x4 viewProjection, Shader* shader, double time);

	void StartTimer(void);
	void ResetTimer(void);

	bool FinishedLoading{ false };

	GraphicalMenu() : thumbnailCount(0), thumbnails(0) {}

private:
	GLuint* thumbnails;
	unsigned int thumbnailCount;

	double tilt_timer{ 1.1 };

	std::vector<ImageData*> thumbnailImages;

	Model quad;
	Model cube;
};

#endif // _GRAPHICALMENU_H