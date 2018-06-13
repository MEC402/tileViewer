#include "Controls.h"

STViewer *Controls::viewer;

// A great deal of this is just wrappers around Camera:: class calls

int Controls::DEBUG_row = 0;
int Controls::DEBUG_col = 0;
float Controls::DEBUG_camerastep = 1.0f;

void Controls::SetViewer(STViewer *v)
{
	viewer = v;
}

void Controls::MouseMove(int posx, int posy)
{
	/*if (Camera::FirstMouse)
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
	Camera::LastY = Camera::Height / 2.0f;*/
}

void Controls::MouseWheel(int button, int direction, int x, int y)
{
	float FOVChange = 0;
	// Zoom in/out by manipulating fov and updating MVP matrix
	if (direction > 0) {
		FOVChange -= 2.0f;
	}
	else {
		FOVChange += 2.0f;
	}
	viewer->MoveCamera(0, 0, FOVChange);
}

// Aside from up/down/right/left these functions are all for debugging
void Controls::ProcessGLUTKeys(int key, int x1, int y1)
{
	float pitchChange = 0;
	float yawChange = 0;
	float FOVChange = 0;
	switch (key) {
	case GLUT_KEY_UP:
		pitchChange += 1.0f;
		break;

	case GLUT_KEY_DOWN:
		pitchChange -= 1.0f;
		break;

	case GLUT_KEY_RIGHT:
		yawChange += 1.0f;
		break;

	case GLUT_KEY_LEFT:
		yawChange -= 1.0f;
		break;

	case GLUT_KEY_PAGE_UP:
		FOVChange += 0.1f;
		break;

	case GLUT_KEY_PAGE_DOWN:
		FOVChange -= 0.1f;
		break;

	case GLUT_KEY_F1:
	case GLUT_KEY_F2:
	case GLUT_KEY_F3:
		viewer->ReloadShaders();
		break;

	case GLUT_KEY_F4:
		//Camera::UpdateCameras();
		break;

	case GLUT_KEY_F5:
		//fprintf(stderr, "FOV is at %f\n", Camera::FOV);
#ifdef DEBUG
		viewer->WaitingThreads();
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
		viewer->FlipDebug();
		break;

	case GLUT_KEY_F9:
		//ImageHandler::WindowDump(Camera::Width, Camera::Height);
		break;
#ifdef DEBUG
	case GLUT_KEY_F10:
		viewer->PrintAverage();
		break;
#endif
	}
	
	viewer->MoveCamera(pitchChange, yawChange, FOVChange);
}

void Controls::ProcessKeys(unsigned char key, int x, int y)
{
	float average = 0.0f;
	switch (key) {
	case '1':
	case '2':
		viewer->ReloadShaders();

#ifdef DEBUG
	case '3':
		viewer->RebindVAO();
		break;
#endif
	case 'f':
		glutFullScreenToggle();
		break;

	case 'h':
		viewer->ToggleStereo();
		break;

	case 'n':
		viewer->NextPano();
		break;

	case 'p':
		viewer->PrevPano();
		break;

	case 'r':
		viewer->ResetCamera();
		break;

	case 'R':
		viewer->ReloadPano();
		break;


	case 27:
		glutLeaveFullScreen();
		exit(0);
		break;
	}
}