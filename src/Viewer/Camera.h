#ifndef CAMERA_H
#define CAMERA_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

class Camera {
public:
	struct Viewport {
		int widthstart;
		int heightstart;
		int width;
		int height;
		float rotation;

		glm::vec3 direction = glm::vec3(0,0,1);
	};

	int NumCameras;
	Viewport **LeftCameras;
	Viewport **RightCameras;
	// Matricies
	glm::mat4 Projection;
	glm::mat4 View;
	glm::mat4 Model;
	glm::mat4 MVP;

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
	float ResetFOV;

	void Init(int cameracount, int width, int height);
	void CreateCameras();
	void SetViewport(Viewport *viewport);
	void SplitHorizontal();
	void UpdateCameras();
	void UpdateMVP();

	Camera() {}

private:
	bool hsplit;

	void createCameras(Viewport **cams, float fovy, float aRatio, bool multiscreen);
	void updateCameras(float fovy, float aRatio, bool hsplit);
	void updateMVP(float pitch, float yaw, float fov, int height, int width);
};

#endif