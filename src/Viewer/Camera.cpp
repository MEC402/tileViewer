#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Camera::Init(int cameracount, int width, int height)
{
	FirstMouse = true;
	Yaw = 0.0f;//-270.0f;
	Pitch = 0.0f;
	LastX = Width / 2.0f;
	LastY = Height / 2.0f;

	if (cameracount < 2) {
		NumCameras = 1;
		FOV = 45.0f;
		Width = width;
		Height = height;
	}
	else {
		NumCameras = cameracount;
		FOV = 34.8093072f; //Magic voodoo number pulled from spviewer codebase
		Width = 1920; // Hardcoded 5-panel width
		Height = 1080; // Hardcode 5-panel height
	}

	cameraUp = glm::vec3(0, 1, 0);
	cameraCenter = glm::vec3(0, 0, 0);

	ResetFOV = FOV;

	LeftCameras = new Viewport*[NumCameras]();
	RightCameras = new Viewport*[NumCameras]();
	UpdateMVP();
	Create();
}

void Camera::Create()
{
	createCameras(LeftCameras, FOV, float(Height) / float(Width), true);
}

void Camera::UpdateMVP()
{
	updateMVP(Pitch, Yaw, FOV, Height, Width);
}

void Camera::UpdateCameras()
{
	updateCameras(FOV, float(Height) / float(Width), hsplit);
}

void Camera::SetViewport(Viewport *viewport)
{
	if (viewport == NULL)
		return;

	float tempYaw = Yaw + glm::degrees(viewport->rotation);
		
	glm::mat4 newView = glm::lookAt(
		cameraCenter,
		glm::vec3(0, 0, 1),
		cameraUp
	);
	
	glm::mat4 rotYaw = glm::rotate(newView, glm::radians(tempYaw), cameraUp);
	glm::vec3 frontVector = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 rightVector = glm::normalize(glm::cross(frontVector, cameraUp));
	glm::mat4 rotPitch = glm::rotate(newView, glm::radians(Pitch), rightVector);
	newView = rotYaw * rotPitch;

	MVP = Projection * newView * Model;

	glViewport(viewport->widthstart, viewport->heightstart, viewport->width, viewport->height);
}

void Camera::SplitHorizontal()
{
	if (hsplit) {
		hsplit = false;
		for (unsigned int i = 0; i < NumCameras; i++)
			delete RightCameras[i];
		delete [] RightCameras;
		for (unsigned int i = 0; i < NumCameras; i++) {
			LeftCameras[i]->height = Height;
			LeftCameras[i]->heightstart = 0;
		}
		return;
	}

	hsplit = true;
	RightCameras = new Viewport*[NumCameras];
	createCameras(RightCameras, FOV, float(1080) / float(1920), true);

	for (unsigned int i = 0; i < NumCameras; i++) {
		LeftCameras[i]->height = Height / 2;
		LeftCameras[i]->heightstart = Height / 2;

		RightCameras[i]->height = Height / 2;
	}
}

/* --------------- Private Functions --------------- */

void Camera::createCameras(Viewport **viewports, float fovy, float aRatio, bool multiscreen)
{
	// Ported from spviewer
	float fovx = glm::atan(glm::tan(glm::radians(fovy*0.5f)) * aRatio) * 2.0f;
	float rotate_x = -(float(NumCameras) - 1) * 0.5f * fovx;

	for (unsigned int i = 0; i < NumCameras; i++) {
		viewports[i] = new Viewport{ 0 };
	}

	// Rotate backwards so our center screen is our "0" rotation camera
	rotate_x -= (fovx * (int)(NumCameras / 2));

	for (unsigned int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
		viewports[i]->widthstart = (Width / NumCameras) * i;
		viewports[i]->heightstart = 0;
		viewports[i]->width = (Width / NumCameras);
		viewports[i]->height = Height;
		viewports[i]->rotation = rotate_x;
	}
}

void Camera::updateCameras(float fovy, float aRatio, bool hsplit)
{
	float fovx = glm::atan(glm::tan(glm::radians(fovy*0.5f)) * aRatio) * 2.0f;
	float rotate_x = -float(NumCameras - 1) * 0.5f * fovx;

	// Rotate backwards so our center screen is our "0" rotation camera
	//rotate_x -= (fovx * (int)(NumCameras / 2));
	// TODO: It's prooobably not very necessary to update EVERYTHING, but its cheap and prevents mistakes so w/e
	if (hsplit) {
		for (unsigned int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
			LeftCameras[i]->width = (Width / NumCameras);
			LeftCameras[i]->widthstart = (Width / NumCameras) * i;
			LeftCameras[i]->height = Height / 2;
			LeftCameras[i]->heightstart = Height / 2;
			LeftCameras[i]->rotation = rotate_x;

			RightCameras[i]->width = (Width / NumCameras);
			RightCameras[i]->widthstart = (Width / NumCameras) * i;
			RightCameras[i]->height = Height / 2;
			RightCameras[i]->heightstart = 0;
			RightCameras[i]->rotation = rotate_x;
		}
	}
	else {
		for (unsigned int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
			LeftCameras[i]->width = (Width / NumCameras);
			LeftCameras[i]->widthstart = (Width / NumCameras) * i;
			LeftCameras[i]->height = Height;
			LeftCameras[i]->heightstart = 0;
			LeftCameras[i]->rotation = rotate_x;
		}
	}
	
}

void Camera::updateMVP(float pitch, float yaw, float fov, int height, int width)
{
	if (NumCameras < 2)
		Projection = glm::perspective(glm::radians(fov), float(width) / float(height), 0.1f, float(height * 2.0f));//10000.0f);
	else
		Projection = glm::perspective(glm::radians(fov), float(1080) / float(1920), 0.1f, float(height * 2.0f));//10000.0f);
	
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	Pitch = pitch;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);

	View = glm::lookAt(
		cameraCenter,
		cameraCenter + cameraFront,
		cameraUp
	);

	Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;
}