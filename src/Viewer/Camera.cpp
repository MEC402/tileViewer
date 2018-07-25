#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Camera::Init(int cameracount, int width, int height)
{

	if (cameracount < 2) {
		NumCameras = 1;
		m_yFOV = 45.0f;
		ViewWidth = ScreenWidth = width;
		ViewHeight = ScreenHeight = height;
		m_aspectRatio = float(ViewWidth) / ViewHeight;
	}
	else if (cameracount == 3) {
		NumCameras = cameracount;
		m_yFOV = 45.0f;
		ViewWidth = width / 3;
		ScreenWidth = width;
		ViewHeight = ScreenHeight = height; 
		m_aspectRatio = float(ViewHeight) / ViewWidth;
	} else {
		NumCameras = cameracount;
		m_yFOV = FivePanelFOV; 
		ViewWidth = FivePanelWidth;
		ViewHeight = FivePanelHeight;
		ScreenWidth = width;
		ScreenHeight = height;
		m_aspectRatio = float(ViewWidth) / ViewHeight;
	}

	cameraUp = glm::vec3(0, 1, 0);
	cameraCenter = glm::vec3(0, 0, 0);
	m_resetFOV = m_yFOV;

	m_yaw = 0.0f;//-270.0f;
	m_pitch = 0.0f;

	LeftCameras = new Viewport*[NumCameras]();
	RightCameras = new Viewport*[NumCameras]();
	UpdateMVP();
	Create();
}

void Camera::Create()
{
	createCameras(LeftCameras, m_yFOV, float(ViewHeight) / float(ViewWidth), true);
}

void Camera::SetPixelPerfect()
{
	float frustumWidth = (NumCameras < 5) ?
		(float(ViewWidth) / tileRes) * tileWidth :
		(float(ViewHeight) / tileRes) * tileWidth;
	float frustumHeight = (NumCameras < 5) ?
		(float(ViewHeight) / tileRes) * tileWidth :
		(float(ViewWidth) / tileRes) * tileWidth;

	float yBaseAngle = glm::degrees(atan((2 * tileDistance) / frustumHeight));
	float xBaseAngle = glm::degrees(atan((2 * tileDistance) / frustumWidth));
	m_yFOV = 180 - 2 * yBaseAngle;
	m_xFOV = 180 - 2 * xBaseAngle;

	UpdateCameras();
	UpdateMVP();
}

void Camera::UpdateMVP()
{
	updateMVP(m_pitch, m_yaw, m_yFOV, ViewHeight, ViewWidth);
}

void Camera::UpdateCameras()
{
	updateCameras(m_yFOV, float(ViewHeight) / float(ViewWidth), hsplit);
}

void Camera::DrawViewport(Viewport *viewport)
{
	std::lock_guard<std::mutex> lock(m_);
	if (viewport == NULL)
		return;

	float tempYaw = m_yaw + glm::degrees(viewport->rotation);
		
	glm::mat4 newView = glm::lookAt(
		cameraCenter,
		glm::vec3(0, 0, 1),
		cameraUp
	);
	
	glm::mat4 rotYaw = glm::rotate(newView, glm::radians(tempYaw), cameraUp);
	glm::vec3 frontVector = glm::normalize(glm::cross(cameraFront, cameraUp));
	glm::vec3 rightVector = glm::normalize(glm::cross(frontVector, cameraUp));
	glm::mat4 rotPitch = glm::rotate(newView, glm::radians(m_pitch), rightVector);

	//newView = glm::lookAt(
	//	cameraCenter,
	//	glm::vec3(0, 0, 1),
	//	cameraUp
	//);
	newView = rotYaw * rotPitch;

	if (viewport->skewViewport) {
		glm::mat4 tempProj = Projection;
		tempProj[2][0] = viewport->viewportHorizontalSkew;
		tempProj[2][1] = viewport->viewportVerticalSkew;
		MVP = tempProj * newView * Model;
	}
	else {
		MVP = Projection * newView * Model;
	}

	glViewport(viewport->widthstart, viewport->heightstart, viewport->width, viewport->height);
}

void Camera::SetViewportOffset(float vertOffset, float horzOffset)
{
	LeftCameras[0]->skewViewport = true;
	LeftCameras[0]->viewportVerticalSkew = vertOffset;
	LeftCameras[0]->viewportHorizontalSkew = horzOffset;
}

void Camera::SplitHorizontal()
{
	if (hsplit) {
		hsplit = false;
		for (unsigned int i = 0; i < NumCameras; i++)
			delete RightCameras[i];
		delete [] RightCameras;
		for (unsigned int i = 0; i < NumCameras; i++) {
			LeftCameras[i]->height = ScreenHeight;
			LeftCameras[i]->heightstart = 0;
		}
		return;
	}

	hsplit = true;
	RightCameras = new Viewport*[NumCameras];
	createCameras(RightCameras, m_yFOV, float(ViewHeight) / float(ViewWidth), true);

	for (unsigned int i = 0; i < NumCameras; i++) {
		LeftCameras[i]->height = ScreenHeight / 2;
		LeftCameras[i]->heightstart = ScreenHeight / 2;

		RightCameras[i]->height = ScreenHeight / 2;
	}
}

void Camera::SetFOV(float FOV)
{
	m_yFOV = FOV;
	setFOVx();
}

void Camera::SetCamera(float exactPitch, float exactYaw)
{
	m_.lock();
	m_pitch = exactPitch;
	m_yaw = exactYaw;
	UpdateMVP();
	UpdateCameras();
	m_.unlock();
}

void Camera::MoveCamera(float pitchDelta, float yawDelta, float FOVDelta)
{
	m_pitch += pitchDelta;
	m_yaw += yawDelta;
	ChangeFOV(FOVDelta);
}

void Camera::ChangeFOV(float delta)
{
	m_yFOV += delta;
	if (m_yFOV < 0.0f)
		m_yFOV = 1.0f;
	if (m_yFOV > 180.0f)
		m_yFOV = 180.0f;
	UpdateMVP();
	UpdateCameras();
	setFOVx();
}

void Camera::ResetCamera()
{
	m_yFOV = m_resetFOV;
	m_pitch = 0.0f;
	UpdateMVP();
	UpdateCameras();
	setFOVx();
}

void Camera::UpdateResolution(int newWidth, int newHeight)
{
	ScreenWidth = newWidth;
	ScreenHeight = newHeight;
	if (NumCameras < 5) {
		ViewWidth = newWidth / NumCameras;
		ViewHeight = newHeight;
	}
	UpdateMVP();
	UpdateCameras();
}

float Camera::GetYaw()
{
	return m_yaw;
}

float Camera::GetPitch()
{
	return m_pitch;
}

void Camera::GetFOV(float &xFOV, float &yFOV)
{
	xFOV = m_xFOV;
	yFOV = m_yFOV;
}

void Camera::SetRotation(float xrotate)
{
	if (NumCameras > 1)
		return UpdateCameras();

	float aRatio = float(ViewWidth) / float(ViewHeight);
	float fovx = glm::atan(glm::tan(glm::radians(xrotate*0.5f)) * aRatio) * 2.0f;
	float rotate_x = -(0.5f * fovx);
	LeftCameras[0]->rotation = rotate_x;
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
		viewports[i]->widthstart = (ScreenWidth / NumCameras) * i;
		viewports[i]->heightstart = 0;
		viewports[i]->width = (ScreenWidth / NumCameras);
		viewports[i]->height = ScreenHeight;
		viewports[i]->rotation = rotate_x;
	}
}

void Camera::updateCameras(float fovy, float aRatio, bool hsplit)
{
	//aRatio = float(ViewWidth) / float(ViewHeight);
	float fovx = glm::atan(glm::tan(glm::radians(fovy*0.5f)) * aRatio) * 2.0f;
	float rotate_x = -float(NumCameras - 1) * 0.5f * fovx;
	
	// TODO: It's prooobably not very necessary to update EVERYTHING, but its cheap and prevents mistakes so w/e
	if (hsplit) {
		for (unsigned int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
			LeftCameras[i]->width = (ScreenWidth / NumCameras);
			LeftCameras[i]->widthstart = (ScreenWidth / NumCameras) * i;
			LeftCameras[i]->height = ScreenHeight / 2;
			LeftCameras[i]->heightstart = ScreenHeight / 2;
			LeftCameras[i]->rotation = rotate_x;

			RightCameras[i]->width = (ScreenWidth / NumCameras);
			RightCameras[i]->widthstart = (ScreenWidth / NumCameras) * i;
			RightCameras[i]->height = ScreenHeight / 2;
			RightCameras[i]->heightstart = 0;
			RightCameras[i]->rotation = rotate_x;
		}
	}
	else {
		for (unsigned int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
			LeftCameras[i]->width = (ScreenWidth / NumCameras);
			LeftCameras[i]->widthstart = (ScreenWidth / NumCameras) * i;
			LeftCameras[i]->height = ScreenHeight;
			LeftCameras[i]->heightstart = 0;
			LeftCameras[i]->rotation = rotate_x;
		}
	}
	
}

void Camera::updateMVP(float pitch, float yaw, float fov, int height, int width)
{
	if (NumCameras < 5)
		Projection = glm::perspective(glm::radians(fov), float(ViewWidth) / float(ViewHeight), 0.1f, float(height * 2.0f));
	else
		Projection = glm::perspective(glm::radians(fov), float(ViewHeight) / float(ViewWidth), 0.1f, float(height * 2.0f));

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	m_pitch = pitch;

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

void Camera::setFOVx()
{
	m_xFOV = glm::degrees(glm::atan(glm::tan(glm::radians(m_yFOV*0.5f)) * m_aspectRatio) * 2.0f);
}