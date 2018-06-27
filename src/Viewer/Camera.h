#ifndef CAMERA_H
#define CAMERA_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
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
		glm::quat cameraQuat = glm::quat(glm::vec3(0, 0, 0));
	};

	unsigned int NumCameras;
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

	// Mouse rotation variables
	bool FirstMouse;
	float LastX;
	float LastY;

	void Init(int cameracount, int width, int height);

	void ChangeFOV(float delta);
	void Create(void);

	void MoveCamera(float pitchChange, float yawChange, float FOVChange);
	void ResetCamera(void);
	void SetPixelPerfect(void);
	void SetViewport(Viewport *viewport);
	void SplitHorizontal(void);

	void UpdateCameras(void);
	void UpdateMVP(void);

	Camera() {}

private:
	const float FivePanelFOV{ 32.8093072f }; //Magic voodoo number pulled from spviewer codebase
	const int FivePanelWidth{ 1920 };
	const int FivePanelHeight{ 1080 };

	const float tileRes{ 512.0f };
	const float tileWidth{ 0.125f };
	const float tileDistance{ 0.5f };

	float m_yFOV;
	float m_xFOV;
	float m_yaw;
	float m_pitch;
	float m_resetFOV;


	bool hsplit;

	glm::vec3 cameraUp;
	glm::vec3 cameraFront;
	glm::vec3 cameraCenter;

	void setFOV(void);
	void createCameras(Viewport **cams, float fovy, float aRatio, bool multiscreen);
	void updateCameras(float fovy, float aRatio, bool hsplit);
	void updateMVP(float pitch, float yaw, float fov, int height, int width);
};

#endif