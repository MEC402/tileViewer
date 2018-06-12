#include "Controls.h"

// A great deal of this is just wrappers around Camera:: class calls

int Controls::DEBUG_row = 0;
int Controls::DEBUG_col = 0;
float Controls::DEBUG_camerastep = 1.0f;
float Controls::DEBUG_fov = Camera::FOV; // So we can reset to our default FOV

void Controls::FlipDebug()
{
	GLuint uDebug = glGetUniformLocation(program, "Debug");
	glUniform1f(uDebug, DEBUG_FLAG);
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
	Camera::UpdateCameras();
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
		break;

	case GLUT_KEY_PAGE_DOWN:
		Camera::FOV -= 0.1f;
		break;

	case GLUT_KEY_F1:
		program = ShaderHelper::ReloadShader(GL_VERTEX_SHADER);
		ImageHandler::RebindTextures(program, 0);
		break;
	case GLUT_KEY_F2:
		program = ShaderHelper::ReloadShader(GL_GEOMETRY_SHADER);
		ImageHandler::RebindTextures(program, 0);
		break;
	case GLUT_KEY_F3:
		program = ShaderHelper::ReloadShader(GL_FRAGMENT_SHADER);
		ImageHandler::RebindTextures(program, 0);
		break;

	case GLUT_KEY_F4:
		Camera::UpdateCameras();
		break;

	case GLUT_KEY_F5:
		fprintf(stderr, "FOV is at %f\n", Camera::FOV);
#ifdef DEBUG
		STViewer::WaitingThreads();
#endif
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
		DEBUG_FLAG = !DEBUG_FLAG;
		FlipDebug();
		break;

	case GLUT_KEY_F9:
		ImageHandler::WindowDump(Camera::Width, Camera::Height);
		break;
#ifdef DEBUG
	case GLUT_KEY_F10:
		STViewer::PrintAverage();
		break;
#endif
	}
	Camera::UpdateMVP();
	//Camera::SetCameras();
}

void Controls::ProcessKeys(unsigned char key, int x, int y)
{
	float average = 0.0f;
	switch (key) {
	case '1':
		ImageHandler::RebindTextures(program, 0);
		break;
	case '2':
		ImageHandler::RebindTextures(program, 1);
		break;

#ifdef DEBUG
	case '3':
		STViewer::RebindVAO();
		break;
#endif
	case 'w':
		break;
	case 'a':
		Camera::Yaw -= 1.0f;
		Camera::UpdateMVP();
		break;
	case 's':
		break;
	case 'd':
		Camera::Yaw += 1.0f;
		Camera::UpdateMVP();
		break;
	case 'W':
		break;
	case 'A':
		Camera::Yaw -= 2.0f;
		Camera::UpdateMVP();
		break;
	case 'S':
		break;
	case 'D':
		Camera::Yaw += 2.0f;
		Camera::UpdateMVP();
		break;

	case 'f':
		glutFullScreenToggle();
		break;

	case 'h':
		STViewer::ToggleStereo();
		break;

	case 'n':
		STViewer::NextPano();
		break;

	case 'p':
		STViewer::PrevPano();
		break;

	case 'r':
		Camera::FOV = DEBUG_fov;
		Camera::Pitch = 0.0f;
		Camera::UpdateMVP();
		Camera::UpdateCameras();
		break;

	case 'R':
		STViewer::ReloadPano();
		break;


	case 27:
		glutLeaveFullScreen();
		exit(0);
		break;
	}
}

void Controls::MainMenu(int choice)
{
	switch (choice) {
	case 1:
		DEBUG_FLAG = !DEBUG_FLAG;
		Controls::FlipDebug();
		break;

	case 2:
		STViewer::NextPano();
		break;

	case 3:
		STViewer::PrevPano();
		break;

	case 4:
		ImageHandler::WindowDump(Camera::Width, Camera::Height);
		break;

	case 5:
		glutFullScreenToggle();
		break;
	}
}

void Controls::PanoMenu(int choice)
{
	STViewer::SelectPano(--choice);
}