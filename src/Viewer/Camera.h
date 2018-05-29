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
		int leftcorner;
		int width;
		int height;
		float rotation;
	};

	static Viewport *Cameras[5];
	// Matricies
	static glm::mat4 Projection;
	static glm::mat4 View;
	static glm::mat4 Model;

	// Defaults for window height/width
	static int Width;
	static int Height;

	// Camera rotation stuff
	static bool FirstMouse;
	static float Yaw;
	static float Pitch;
	static float LastX;
	static float LastY;
	static float Zoom;
	static float FOV;

	static void SetViewport(Viewport *viewport);
	static void SetCameras();
	static void UpdateMVP();


private:
	static void setCameras(Viewport **cams, float fovy, float aRatio, bool multiscreen);
	static void updateMVP(float pitch, float yaw, float fov, int height, int width);
};

#endif