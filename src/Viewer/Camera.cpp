#include "Camera.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

void Camera::Init(int cameracount, int width, int height)
{
	//frustum = glm::frustum(left, right, bottom, top, _near, _far);
	Model = glm::mat4(1.0f);
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

	//LeftCameras = new Viewport*[NumCameras]();
	LeftCameras = new Viewport*[3]();
	RightCameras = new Viewport*[NumCameras]();
	UpdateMVP();
	Create();
	SetPixelPerfect();
	//OffsetFrustum(0, 0, 0, 0, 0, 0);

	LeftCameras[1] = new Viewport{ 0 };
	LeftCameras[1]->width = LeftCameras[0]->width;
	LeftCameras[1]->height = LeftCameras[0]->height;
	LeftCameras[1]->widthstart = 0;
	LeftCameras[1]->heightstart = height / 2;
	LeftCameras[1]->rotation = 0;
	LeftCameras[1]->skewViewport = true;
	LeftCameras[1]->viewportVerticalSkew = 2.0f;

	LeftCameras[2] = new Viewport{ 0 };
	LeftCameras[2]->width = LeftCameras[0]->width;
	LeftCameras[2]->height = LeftCameras[0]->height;
	LeftCameras[2]->widthstart = LeftCameras[0]->width/2;
	LeftCameras[2]->heightstart = 0;
	//LeftCameras[2]->rotation = glm::radians(0.5f * m_xFOV);
	LeftCameras[2]->skewViewport = true;
	LeftCameras[2]->viewportHorizontalSkew = 2.0f;
}

void Camera::Create()
{
	createCameras(LeftCameras, m_yFOV, float(ViewWidth) / float(ViewHeight), true);
}

void Camera::SetPixelPerfect()
{
	float frustumWidth = (float(ViewWidth) / tileRes) * tileWidth;
	float frustumHeight = (float(ViewHeight) / tileRes) * tileWidth;

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
	updateCameras(m_yFOV, float(ViewWidth) / float(ViewHeight), hsplit);
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

	newView = glm::lookAt(
		cameraCenter,
		glm::vec3(0, 0, 1),
		cameraUp
	);
	newView *= rotYaw * rotPitch;

	if (viewport->skewViewport) {
		glm::mat4 tempProj = Projection;
		tempProj[2][0] = viewport->viewportHorizontalSkew; // 2.0 at 4.1 yFOV
		tempProj[2][1] = viewport->viewportVerticalSkew; // 2.0 at 4.1 yFOV
		MVP = tempProj * newView * Model;
	}
	else {
		MVP = Projection * newView * Model;
	}

	glViewport(viewport->widthstart, viewport->heightstart, viewport->width/2, viewport->height/2);
}

void Camera::OffsetFrustum(float leftDelta, float rightDelta, float topDelta, float bottomDelta, float nearDelta, float farDelta)
{
	left += leftDelta;
	right += rightDelta;
	top += topDelta;
	bottom += bottomDelta;
	_near += nearDelta;
	_far += farDelta;

	//float wd = _near * tan(glm::radians(m_xFOV) / 2);
	//glm::vec3 rightVec = glm::cross(cameraFront, cameraUp);
	//float t = wd + top;
	//float b = -wd + bottom;
	//float l = -m_aspectRatio * wd + left;
	//float r = m_aspectRatio * wd + right;

	//frustum = glm::frustum(l, r, b, t, _near, _far);

	float tanHalfFovy = tan(glm::radians(m_yFOV) / 2);
	frustum = glm::mat4(
		glm::vec4(1.0f / (m_aspectRatio * tanHalfFovy), 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f / (tanHalfFovy), 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, -(_far + _near) / (_far - _near), -1.0f),
		glm::vec4(0.0f, 0.0f, -(2.0f * _far * _near) / (_far - _near), 0.0f)
	);

	
	//frustum[0][0] = 1.0f / (m_aspectRatio * tanHalfFovy);
	//frustum[1][1] = 1.0f / (tanHalfFovy);
	//frustum[2][2] = -(_far + _near) / (_far - _near);
	//frustum[2][3] = -1.0f;
	//frustum[3][2] = -(2.0f * _far * _near) / (_far - _near);

	UpdateMVP();
	UpdateCameras();
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
//	OffsetFrustum(0, 0, 0, 0, 0, 0);

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
	//SetPixelPerfect();
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
	//aRatio = float(ViewWidth) / float(ViewHeight);
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
	
	// Rotate backwards so our center screen is our "0" rotation camera
	//rotate_x -= (fovx * (int)(NumCameras / 2));
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
	//if (NumCameras < 5)
	Projection = glm::perspective(glm::radians(fov), float(ViewWidth) / float(ViewHeight), 0.1f, float(height * 2.0f));//10000.0f);
	//Projection = glm::perspectiveFov(glm::radians(fov), float(1.0), float(1.0), 0.1f, 1000.0f);
	//Projection = frustum;
	//else
	//	Projection = glm::perspective(glm::radians(fov), float(ViewHeight) / float(ViewWidth), 0.1f, float(height * 2.0f));//10000.0f);

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

	//Model = glm::mat4(1.0f);
	MVP = Projection * View * Model;
}

void Camera::setFOVx()
{
	m_xFOV = glm::degrees(glm::atan(glm::tan(glm::radians(m_yFOV*0.5f)) * m_aspectRatio) * 2.0f);
}