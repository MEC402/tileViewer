#ifndef CAMERA_H
#define CAMERA_H

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <stdio.h>
#include <mutex>

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

	// Viewport height/width
	int ViewWidth;
	int ViewHeight;

	// Resolution height/width
	int ScreenWidth;
	int ScreenHeight;

	// Mouse rotation variables
	bool FirstMouse;
	float LastX;
	float LastY;

	void Init(int cameracount, int width, int height);

	void SetFOV(float FOV);
	void ChangeFOV(float delta);
	void Create(void);

	void SetCamera(float exactPitch, float exactYaw);
	void MoveCamera(float pitchChange, float yawChange, float FOVChange);
	void ResetCamera(void);
	void SetPixelPerfect(void);
	void SplitHorizontal(void);

	void DrawViewport(Viewport *viewport);

	void UpdateResolution(int newWidth, int newHeight);
	void UpdateCameras(void);
	void UpdateMVP(void);

	float GetYaw(void);
	float GetPitch(void);
	void GetFOV(float &xFOV, float &yFOV);

	Camera() {}

private:
	const float FivePanelFOV{ 32.8093072f }; //Magic voodoo number pulled from spviewer codebase
	const int FivePanelWidth{ 1080 };
	const int FivePanelHeight{ 1920 };

	const float tileRes{ 512.0f };
	const float tileWidth{ 0.125f };
	const float tileDistance{ 0.5f };

	float m_yFOV;
	float m_xFOV;
	float m_yaw;
	float m_pitch;
	float m_resetFOV;
	float m_aspectRatio;

	bool hsplit;

	std::mutex m_;

	glm::vec3 cameraUp;
	glm::vec3 cameraFront;
	glm::vec3 cameraCenter;

	void setFOVx(void);
	void createCameras(Viewport **cams, float fovy, float aRatio, bool multiscreen);
	void updateCameras(float fovy, float aRatio, bool hsplit);
	void updateMVP(float pitch, float yaw, float fov, int height, int width);
};

#endif