#include "Controls.h"

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

	case 'h':
		Camera::SplitHorizontal();
		break;

	case 'n':
		LeftEye->ResetDepth();
		if (stereo)
			RightEye->ResetDepth();
		ImageHandler::InitPanoListFromOnlineFile(panolist.front());
		panolist.pop();
		for (int i = 0; i < 6; i++) {
			ImageHandler::LoadQuadImage(i, 0, 0, 0, 0);
			facedepths[0][i] = 1;
			facecount[0][i] = 0;
			if (stereo) {
				ImageHandler::LoadQuadImage(i, 0, 0, 0, 1);
				facedepths[1][i] = 1;
				facecount[1][i] = 0;
			}
		}
		break;

	case 'r':
	case 'R':
		Camera::FOV = DEBUG_fov;
		Camera::Pitch = 0.0f;
		Camera::UpdateMVP();
		//Camera::SetCameras();
		break;

	case 27:
		//glutLeaveFullScreen();
		glutLeaveGameMode();
		break;
	}
}