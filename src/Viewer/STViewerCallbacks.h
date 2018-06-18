// STViewerCallbacks is for GLUT callback and init functions, as the API will not accept memeber functions
// This file should not be included anywhere except STViewer.cpp
#include "STViewer.h"
#include "Controls.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

STViewer *_viewer;

void _UpdateEyes(CubePoints *lefteye, CubePoints *righteye, bool stereo);
void _InitReferences(bool &stereo, Shader *shader, ImageHandler *images, CubePoints *lefteye, CubePoints *righteye, Camera *camera);
void _InitCallbacks(STViewer *v, bool fullscreen);
void _EnableVR(VRDevice *vr);
void _InitMenus(std::vector<PanoInfo> &panoList);
void _MainMenu(int choice);
void _PanoMenu(int choice);
void _Cleanup(void);
void _Display(void);
void _Idle(void);
void _Resize(int width, int height);

int _Width = 1280;
int _Height = 800;

bool _usingVR = false;
VRDevice *_vr;
double _globalTime;
long long _programStartTime;
bool _stereo = false;
Shader *_shader;
ImageHandler *_images;
CubePoints *_lefteye;
CubePoints *_righteye;
Camera *_camera;



void _UpdateEyes(CubePoints *lefteye, CubePoints *righteye, bool stereo)
{
	_lefteye = lefteye;
	_righteye = righteye;
	_stereo = stereo;
}

void _InitReferences(bool &stereo, Shader *shader, ImageHandler *images, 
	CubePoints *lefteye, CubePoints *righteye, Camera *camera)
{
	_stereo = stereo;
	_shader = shader;
	_images = images;
	_lefteye = lefteye;
	if (_stereo)
		_righteye = righteye;
	_camera = camera;

	// Init time
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	_programStartTime = time.QuadPart;
}

void _EnableVR(VRDevice *vr)
{
	_stereo = true;
	_usingVR = true;
	_vr = vr;
}

void _InitCallbacks(STViewer *v, bool fullscreen)
{
	_viewer = v;

	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(_Width, _Height); // Defaults to 1280 x 800 windowed
	glutCreateWindow("TileViewer - ST Shader Annihilation Edition");
	if (fullscreen) {
		glutFullScreen();
	}

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Setup callbacks
	atexit(_Cleanup);
	glutDisplayFunc(_Display);
	glutIdleFunc(_Idle);
	glutReshapeFunc(_Resize);
	glutSpecialFunc(Controls::ProcessGLUTKeys);
	glutKeyboardFunc(Controls::ProcessKeys);
	//glutMotionFunc(Controls::MouseMove); // This is super broken with 5-panel displays, just disable it.
	glutMouseWheelFunc(Controls::MouseWheel);
	//glutTimerFunc(5000, timerCleanup, 0);
}

void _InitMenus(std::vector<PanoInfo> &panoList)
{
	int panomenu = glutCreateMenu(_PanoMenu);
	for (unsigned int i = 0; i < panoList.size(); i++) {
		char buf[64];
		sprintf_s(buf, "%s", panoList[i].displayName.c_str());
		glutAddMenuEntry(buf, i + 1);
	}

	int mainmenu = glutCreateMenu(_MainMenu);
	glutAddMenuEntry("Toggle ST scaling (F8)", 1);
	glutAddSubMenu("Pano Select", panomenu);
	glutAddMenuEntry("Next Pano (n)", 2);
	glutAddMenuEntry("Prev Pano (p)", 3);
	glutAddMenuEntry("Screenshot (F9)", 4);
	glutAddMenuEntry("Toggle Fullscreen (f)", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void _MainMenu(int choice)
{
	switch (choice) {
	case 1:
		DEBUG_FLAG = !DEBUG_FLAG;
		_viewer->FlipDebug();
		break;

	case 2:
		_viewer->NextPano();
		break;

	case 3:
		_viewer->PrevPano();
		break;

	case 4:
		_viewer->Screenshot();
		break;

	case 5:
		glutFullScreenToggle();
		break;
	}
}

void _PanoMenu(int choice)
{
	_viewer->SelectPano(--choice);
}

void _Cleanup()
{
	_viewer->Cleanup();
}

void _Display()
{
	if (_usingVR) {
		glDisable(GL_CULL_FACE);
		for (unsigned int eyeIndex = 0; eyeIndex < 2; ++eyeIndex) {
			_shader->Bind();
			_shader->SetFloatUniform("TileWidth", _lefteye->m_TILEWIDTH);

			bindEyeRenderSurface(_vr, eyeIndex);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			_images->BindTextures(*_shader, eyeIndex);

			glm::mat4x4 perspective = buildVRProjectionMatrix(_vr, eyeIndex);
			glm::mat4x4 view = buildVRViewMatrix(_vr, eyeIndex, 0, 0, 0);
			view = glm::translate(view, getVRHeadsetPosition(_vr)); // Negate headset translation
			_shader->SetMatrixUniform("MVP", perspective*view);

			if (eyeIndex == 0) {
				_lefteye->BindVAO();
				glDrawArrays(GL_POINTS, 0, _lefteye->m_NumVertices);
			}
			else {
				_righteye->BindVAO();
				glDrawArrays(GL_POINTS, 0, _righteye->m_NumVertices);
			}


			double uiDisplayWaitTime = 1.5;
			if (_globalTime - _viewer->m_lastUIInteractionTime < uiDisplayWaitTime) {
				glm::mat4x4 inverseView = (glm::mat4_cast(getVRHeadsetRotation(_vr)));
				float uiRadius = 0.65;
				_viewer->m_gui.display(getVRHeadsetRotation(_vr), perspective*view, uiRadius, _viewer->m_guiPanoSelection);
			}

			commitEyeRenderSurface(_vr, eyeIndex);
		}
		finishVRFrame(_vr);
		blitHeadsetView(_vr, 0);
	} 
	else
	{
		_shader->Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glBindVertexArray(_lefteye->m_PositionVAOID);
		for (int i = 0; i < _camera->NumCameras; i++) {
			_camera->SetViewport(_camera->LeftCameras[i]);
			_shader->SetMatrixUniform("MVP", _camera->MVP);
			glDrawArrays(GL_POINTS, 0, _lefteye->m_NumVertices);
		}

		if (_stereo) {
			glBindVertexArray(_righteye->m_PositionVAOID);
			_images->BindTextures(*_shader, 1);
			for (int i = 0; i < _camera->NumCameras; i++) {
				_camera->SetViewport(_camera->RightCameras[i]);
				_shader->SetMatrixUniform("MVP", _camera->MVP);
				glDrawArrays(GL_POINTS, 0, _righteye->m_NumVertices);
			}
			_images->BindTextures(*_shader, 0);
		}

		glFlush();
	}

#ifdef DEBUG
	PRINT_GL_ERRORS
#endif
}

void _Idle()
{
	LARGE_INTEGER time;
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&ticksPerSecond);
	float deltaTime = float(time.QuadPart - _programStartTime) / float(ticksPerSecond.QuadPart) - _globalTime;
	_globalTime = (time.QuadPart - _programStartTime) / double(ticksPerSecond.QuadPart);

	_viewer->Update(_globalTime, deltaTime);
}

void _Resize(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	_camera->Width = w;
	_camera->Height = h;
#ifdef DEBUG
	fprintf(stderr, "%d %d\n", _camera->Width, _camera->Height);
#endif
	_camera->UpdateMVP();
	_camera->UpdateCameras();

	if (_usingVR) {
		resizeMirrorTexture(_vr, w, h);
	}
}