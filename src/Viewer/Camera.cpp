#include "Camera.h"


int Camera::Width = 1280;
int Camera::Height = 800;

int Camera::NumCameras = 5;
Camera::Viewport **Camera::Cameras = new Viewport*[NumCameras];

glm::mat4 Camera::Projection;
glm::mat4 Camera::View;
glm::mat4 Camera::Model;

// Camera rotation stuff
bool Camera::FirstMouse = true;
float Camera::Yaw = -270.0f;
float Camera::Pitch = 0.0f;
float Camera::LastX = Width / 2.0f;
float Camera::LastY = Height / 2.0f;
float Camera::Zoom = 0.0f;
float Camera::FOV = 34.8093072f;

void Camera::SetCameras()
{
	setCameras(Cameras, FOV, float(Height) / float(Width), true);
}

void Camera::UpdateMVP()
{
	updateMVP(Pitch, Yaw, FOV, Height, Width);
}

void Camera::SetViewport(Viewport *viewport)
{
	glm::mat4 newModel = glm::rotate(Model, viewport->rotation, glm::vec3(0, 1, 0));
	glm::mat4 mvp = Projection * View * newModel;
	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	if (MatrixID != -1) {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	}
	else {
		GLenum error = glGetError();
		fprintf(stderr, "Error occured trying to update MVP uniform!\n%s\n", gluErrorString(error));
	}

	glViewport(viewport->leftcorner, 0, viewport->width, viewport->height);
	glDrawArrays(GL_POINTS, 0, pointCount);
	GLenum error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf(stderr, "Error occured trying to render viewport!\n%s\n", gluErrorString(error));
	}
}

void Camera::setCameras(Viewport **viewports, float fovy, float aRatio, bool multiscreen)
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

	for (unsigned int i = 0; i < numScreens; ++i, rotate_x += fovx) {
		//fprintf(stderr, "Camera %d %d\n", i, (Width / numScreens));
		viewports[i]->leftcorner = (Width / numScreens) * i;
		viewports[i]->width = (Width / numScreens);
		viewports[i]->height = Height;
		viewports[i]->rotation = rotate_x;
	}
}

void Camera::updateMVP(float pitch, float yaw, float fov, int height, int width)
{
	Projection = glm::perspective(glm::radians(fov), (float)(float(height) / float(width)), 0.1f, 10000.0f);
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