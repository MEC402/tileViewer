#include "Camera.h"


int Camera::Width = 1280;
int Camera::Height = 800;

int Camera::NumCameras;
Camera::Viewport **Camera::LeftCameras;
Camera::Viewport **Camera::RightCameras;

bool Camera::hsplit = false;

glm::mat4 Camera::Projection;
glm::mat4 Camera::View;
glm::mat4 Camera::Model;

// Camera rotation stuff
bool Camera::FirstMouse = true;
float Camera::Yaw = -270.0f;
float Camera::Pitch = 0.0f;
float Camera::LastX = Width / 2.0f;
float Camera::LastY = Height / 2.0f;
//float Camera::FOV = 34.8093072f; //Magic voodoo number pulled from spviewer codebase
float Camera::FOV = 73.74f;

void Camera::Init(int cameracount)
{
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
	createCameras(LeftCameras, FOV, float(Height) / float(Width), true);
	//createCameras(LeftCameras, FOV, float(Width / NumCameras) / float(Height), true);
	//createCameras(LeftCameras, FOV, float(1080) / float(1920), true);
}

void Camera::UpdateMVP()
{
	updateMVP(Pitch, Yaw, FOV, Height, Width);
}

void Camera::UpdateCameras()
{
	updateCameras(FOV, float(Height) / float(Width), hsplit);
	//updateCameras(FOV, float(Width / NumCameras) / float(Height), hsplit);
	//updateCameras(FOV, float(1080) / float(1920), hsplit);
}

void Camera::SetViewport(Viewport *viewport)
{
	if (viewport == NULL)
		return;
	// rotation in degrees or radians?
	glm::mat4 newView = glm::rotate(View, viewport->rotation, glm::vec3(0, 1, 0));
	glm::mat4 mvp = Projection * newView * Model;
	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	if (MatrixID != -1) {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	}
	else {
		GLenum error = glGetError();
		fprintf(stderr, "Error occured trying to update MVP uniform!\n%s\n", gluErrorString(error));
	}

	glViewport(viewport->widthstart, viewport->heightstart, viewport->width, viewport->height);
	glDrawArrays(GL_POINTS, 0, pointCount);

	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf(stderr, "Error occured trying to render viewport!\n%s\n", gluErrorString(error));
	}
}

void Camera::SplitHorizontal()
{
	if (hsplit)
		return;

	hsplit = true;
	RightCameras = new Viewport*[NumCameras];
	createCameras(RightCameras, FOV, float(Height) / float(Width), true);

	for (int i = 0; i < NumCameras; i++) {
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
	unsigned int numScreens = 1;
	float rotate_x = 0.0;
	if (multiscreen) {
		numScreens = NumCameras;
		rotate_x = -float(numScreens - 1) * 0.5f * fovx;
		for (unsigned int i = 0; i < numScreens; i++) {
			viewports[i] = new Viewport{ 0 };
		}
	}
	else {
		viewports = new Viewport*[1];
		viewports[0] = new Viewport{ 0 };
	}
	rotate_x -= (fovx * (int)(numScreens / 2));
	for (unsigned int i = 0; i < numScreens; ++i, rotate_x += fovx) {
		//fprintf(stderr, "Camera %d %d\n", i, (Width / numScreens));
		viewports[i]->widthstart = (Width / numScreens) * i;
		viewports[i]->heightstart = 0;
		viewports[i]->width = (Width / numScreens);
		viewports[i]->height = Height;
		viewports[i]->rotation = rotate_x;
		//fprintf(stderr, "%d rotation at %f\n", i, rotate_x);
	}
}

void Camera::updateCameras(float fovy, float aRatio, bool hsplit)
{
	float fovx = glm::atan(glm::tan(glm::radians(fovy*0.5f)) * aRatio) * 2.0f;
	float rotate_x = -(fovx * (int)(NumCameras / 2));


	if (hsplit) {
		for (int i = 0; i < NumCameras; ++i, rotate_x += fovx) {
			LeftCameras[i]->rotation = rotate_x;
			RightCameras[i]->rotation = rotate_x;
		}
	}
	else {
		for (int i = 0; i < NumCameras; ++i, rotate_x += fovx)
			LeftCameras[i]->rotation = rotate_x;
	}
	
}

void Camera::updateMVP(float pitch, float yaw, float fov, int height, int width)
{
	if (NumCameras < 2) {
		Projection = glm::perspective(glm::radians(fov), (float)(float(width) / float(height)), 0.1f, 10000.0f);
	}
	else {
		Projection = glm::perspective(glm::radians(fov), (float)(float(height) / float(width)), 0.1f, 10000.0f);
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

	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	if (MatrixID != -1) {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	}
	else {
		GLenum error = glGetError();
		fprintf(stderr, "Error occured trying to set MVP uniform!\n%s\n", gluErrorString(error));
	}
}