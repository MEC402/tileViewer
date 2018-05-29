#include "stdafx.h"
#include "Controls.h"
#include "Camera.h"

// A great deal of this is just wrappers around Camera:: class calls

bool Controls::DEBUG = false;
int Controls::DEBUG_row = 0;
int Controls::DEBUG_col = 0;
float Controls::DEBUG_camerastep = 1.0f;
float Controls::DEBUG_fov = 34.8093072;
float Controls::DEBUG_camera_degree_shift[5];

void Controls::FlipDebug()
{
	GLuint uDebug = glGetUniformLocation(program, "Debug");
	glUniform1f(uDebug, DEBUG);
}

void Controls::MouseMove(int posx, int posy)
{
	if (Camera::FirstMouse)
	{
		Camera::LastX = (float)posx;
		Camera::LastY = (float)posy;
		Camera::FirstMouse = false;
	}

	float xoffset = posx - Camera::LastX;
	float yoffset = Camera::LastY - posy; // reversed since y-coordinates go from bottom to top
	Camera::LastX = (float)posx;
	Camera::LastY = (float)posy;

	float sensitivity = 0.01f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	Camera::Yaw += xoffset;
	Camera::Pitch += yoffset;
	Camera::UpdateMVP();
	Camera::LastX = Camera::Width / 2.0f;
	Camera::LastY = Camera::Height / 2.0f;
}

void Controls::MouseWheel(int button, int direction, int x, int y)
{
	// Zoom in/out by manipulating fov and updating MVP matrix
	if (direction > 0)
	{
		Camera::FOV -= 2.0f;
		if (Camera::FOV < 0) {
			Camera::FOV = 1.0f;
		}
	}
	else
	{
		Camera::FOV += 2.0f;
	}
	Camera::UpdateMVP();
	Camera::SetCameras();
}

// Aside from up/down/right/left these functions are all for debugging
void Controls::ProcessGLUTKeys(int key, int x1, int y1)
{
	switch (key) {
	case GLUT_KEY_UP:
		Camera::Pitch += 1.0f;
		break;

	case GLUT_KEY_DOWN:
		Camera::Pitch -= 1.0f;
		break;

	case GLUT_KEY_RIGHT:
		Camera::Yaw += 1.0f;
		break;

	case GLUT_KEY_LEFT:
		Camera::Yaw -= 1.0f;
		break;

	case GLUT_KEY_PAGE_UP:
		Camera::FOV += 0.1f;
		//if (DEBUG_row >= 7) {
		//	DEBUG_row = 0;
		//}
		//else {
		//	DEBUG_row++;
		//}
		//fprintf(stderr, "Select row/col to increase depth: %d / %d\n", DEBUG_row, DEBUG_col);
		break;

	case GLUT_KEY_PAGE_DOWN:
		Camera::FOV -= 0.1f;
		//if (DEBUG_col >= 7) {
		//	DEBUG_col = 0;
		//}
		//else {
		//	DEBUG_col++;
		//}
		//fprintf(stderr, "Select row/col to increase depth: %d / %d\n", DEBUG_row, DEBUG_col);
		break;

	case GLUT_KEY_F1:
		program = ShaderHelper::ReloadShader(GL_VERTEX_SHADER);
		ImageHandler::RebindTextures(program);
		break;
	case GLUT_KEY_F2:
		program = ShaderHelper::ReloadShader(GL_GEOMETRY_SHADER);
		ImageHandler::RebindTextures(program);
		break;
	case GLUT_KEY_F3:
		program = ShaderHelper::ReloadShader(GL_FRAGMENT_SHADER);
		ImageHandler::RebindTextures(program);
		break;

	case GLUT_KEY_F4:
		Camera::SetCameras();
		break;

	case GLUT_KEY_F5:
		fprintf(stderr, "FOV is at %f\n", Camera::FOV);
		for (int i = 0; i < 5; i++) {
			fprintf(stderr, "Camera %d is shifted %f degrees\n", i, DEBUG_camera_degree_shift[i]);
		}
		break;

	case GLUT_KEY_F6:
		DEBUG_camerastep -= 0.1f;
		fprintf(stderr, "Stepping now at %f\n", DEBUG_camerastep);
		break;

	case GLUT_KEY_F7:
		DEBUG_camerastep += 0.1f;
		fprintf(stderr, "Stepping now at %f\n", DEBUG_camerastep);
		break;

	case GLUT_KEY_F8:
		DEBUG = !DEBUG;
		FlipDebug();
		break;

	case GLUT_KEY_F9:
		Camera::UpdateMVP();
		break;

	case GLUT_KEY_F10:
		//zoffset -= step;
		break;

	case GLUT_KEY_F12:
		//Threads::DefaultThreadPool::submitJob(LoadFaceByQuads, 0);
		break;
	}
}

void Controls::ProcessKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		DEBUG_camera_degree_shift[0] += DEBUG_camerastep;
		break;
	case '2':
		DEBUG_camera_degree_shift[0] -= DEBUG_camerastep;
		break;
	case '3':
		DEBUG_camera_degree_shift[1] += DEBUG_camerastep;
		break;
	case '4':
		DEBUG_camera_degree_shift[1] -= DEBUG_camerastep;
		break;
	case '5':
		DEBUG_camera_degree_shift[2] += DEBUG_camerastep;
		break;
	case '6':
		DEBUG_camera_degree_shift[2] -= DEBUG_camerastep;
		break;
	case '7':
		DEBUG_camera_degree_shift[3] += DEBUG_camerastep;
		break;
	case '8':
		DEBUG_camera_degree_shift[3] -= DEBUG_camerastep;
		break;
	case '9':
		DEBUG_camera_degree_shift[4] += DEBUG_camerastep;
		break;
	case '0':
		DEBUG_camera_degree_shift[4] -= DEBUG_camerastep;
		break;

	case 'r':
	case 'R':
		Camera::FOV = DEBUG_fov;
		Camera::Pitch = 0.0f;
		Camera::UpdateMVP();
		Camera::SetCameras();
		break;

	case 27:
		//glutLeaveFullScreen();
		glutLeaveGameMode();
		break;
	}
}