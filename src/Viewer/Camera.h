#ifndef CAMERA_H
#define CAMERA_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#include "GLhandles.h"

class Camera {
public:
	struct Viewport {
		int widthstart;
		int heightstart;
		int width;
		int height;
		float rotation;
	};

	int NumCameras;
	Viewport **LeftCameras;
	Viewport **RightCameras;
	// Matricies
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;

	// Defaults for window height/width
	int Width;
	int Height;

	// Camera rotation stuff
	bool FirstMouse;
	float Yaw;
	float Pitch;
	float LastX;
	float LastY;
	float Zoom;
	float FOV;

	void Init(int cameracount);
	void CreateCameras();
	void SetViewport(Viewport *viewport);
	void SplitHorizontal();
	void UpdateCameras();
	void UpdateMVP();
	void setShader(GLuint shader);

	Camera() {}

private:
	bool hsplit;
	GLuint m_shader;

	void createCameras(Viewport **cams, float fovy, float aRatio, bool multiscreen);
	void updateCameras(float fovy, float aRatio, bool hsplit);
	void updateMVP(float pitch, float yaw, float fov, int height, int width);
};

#endif