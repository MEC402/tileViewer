#include "Camera.h"


void Camera::Init(int cameracount, int width, int height)
{
	Width = width;
	Height = height;

	FirstMouse = true;
	Yaw = -270.0f;
	Pitch = 0.0f;
	LastX = Width / 2.0f;
	LastY = Height / 2.0f;
	FOV = 34.8093072f; //Magic voodoo number pulled from spviewer codebase

	if (cameracount < 2) {
		NumCameras = 1;
		FOV = 45.0f;
	}
	else {
		NumCameras = cameracount;
	}
	LeftCameras = new Viewport*[NumCameras]();
	RightCameras = new Viewport*[NumCameras]();
	UpdateMVP();
	CreateCameras();
}

void Camera::CreateCameras()
{
	createCameras(LeftCameras, FOV, float(1080) / float(1920), true);
}

void Camera::UpdateMVP()
{
	updateMVP(Pitch, Yaw, FOV, Height, Width);
}

void Camera::UpdateCameras()
{
	updateCameras(FOV, float(1080) / float(1920), hsplit);
}

void Camera::SetViewport(Viewport *viewport)
{
	if (viewport == NULL)
		return;
	// rotation in degrees or radians?
	glm::vec3 front;
	front.x = cos(glm::radians(Yaw + glm::degrees(viewport->rotation))) * cos(glm::radians(Pitch));
	front.y = sin(glm::radians(Pitch));
	front.z = sin(glm::radians(Yaw + glm::degrees(viewport->rotation))) * cos(glm::radians(Pitch));
	glm::vec3 cameraFront = glm::normalize(front);

	glm::mat4 newView = glm::lookAt(
		glm::vec3(0, 0, 0),
		cameraFront,
		glm::vec3(0, 1, 0)
	);

	//glm::mat4 newView = glm::rotate(View, viewport->rotation, glm::vec3(0, 1, 0));
	glm::mat4 mvp = Projection * newView * Model;

	GLuint MatrixID = glGetUniformLocation(m_shader, "MVP");
	if (MatrixID != -1) {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	}
	else {
		fprintf(stderr, "Error occured trying to update MVP uniform!\n%s\n", gluErrorString(glGetError()));
	}

	glViewport(viewport->widthstart, viewport->heightstart, viewport->width, viewport->height);
}

void Camera::SplitHorizontal()
{
	if (hsplit) {
		hsplit = false;
		for (int i = 0; i < NumCameras; i++)
			delete RightCameras[i];
		delete [] RightCameras;
		for (int i = 0; i < NumCameras; i++) {
			LeftCameras[i]->height = Height;
			LeftCameras[i]->heightstart = 0;
		}
		return;
	}

	hsplit = true;
	RightCameras = new Viewport*[NumCameras];
	createCameras(RightCameras, FOV, float(1080) / float(1920), true);

	for (int i = 0; i < NumCameras; i++) {
		LeftCameras[i]->height = Height / 2;
		LeftCameras[i]->heightstart = Height / 2;

		RightCameras[i]->height = Height / 2;
	}
}

void Camera::setShader(GLuint shader)
{
	m_shader = shader;
}

/* --------------- Private Functions --------------- */

void Camera::createCameras(Viewport **viewports, float fovy, float aRatio, bool multiscreen)
{
	// Ported from spviewer
	float fovx = glm::atan(glm::tan(glm::radians(fovy*0.5f)) * aRatio) * 2.0f;

	float rotate_x = -float(NumCameras - 1) * 0.5f * fovx;
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
	float rotate_x = -(fovx * (int)(NumCameras / 2));

	// TODO: It's prooobably not very necessary to update EVERYTHING, but its cheap so w/e
	if (hsplit) {
		for (int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
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
		for (int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
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
	if (NumCameras < 2) {
		Projection = glm::perspective(glm::radians(fov), float(width) / float(height), 0.1f, 10000.0f);
	}
	else {
		//Projection = glm::perspective(glm::radians(fov), float(height) / (float(width) / float(NumCameras)), 0.1f, 10000.0f);
		//Projection = glm::perspective(glm::radians(fov), float(height) / float(width), 0.1f, 10000.0f);
		Projection = glm::perspective(glm::radians(fov), float(1080)/float(1920), 0.1f, 10000.0f);
	}
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	glm::vec3 cameraFront = glm::normalize(front);

	View = glm::lookAt(
		glm::vec3(0, 0, 0),
		cameraFront,
		glm::vec3(0, 1, 0)
	);

	Model = glm::mat4(1.0f);

	glm::mat4 mvp = Projection * View * Model;

	GLuint MatrixID = glGetUniformLocation(m_shader, "MVP");
	if (MatrixID != -1) {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	}
	else {
		GLenum error = glGetError();
		fprintf(stderr, "Error occured trying to set MVP uniform!\n%s\n", gluErrorString(error));
	}
}