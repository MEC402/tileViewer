// STViewerCallbacks is for GLUT callback and init functions, as the API will not accept memeber functions
// This file should not be included anywhere except STViewer.cpp
#include "STViewer.h"
#include "Controls.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/euler_angles.hpp"

STViewer *_viewer;

// CB = Callback, meant to indicate "Safe to call directly"
void CB_UpdateEyes(CubePoints *lefteye, CubePoints *righteye, bool stereo);
void CB_InitReferences(bool &stereo, Shader *shader, ImageHandler *images, CubePoints *lefteye, CubePoints *righteye, Camera *camera);
void CB_Init(STViewer *v, bool fullscreen);
void CB_EnableVR(VRDevice *vr);
void CB_InitMenus(std::vector<PanoInfo> &panoList);
void CB_Display(void);

// Don't preface with CB to indicate "don't call these directly"
void _MainMenu(int choice); 
void _PanoMenu(int choice); 
void _Cleanup(void);
void _Idle(void);
void _Resize(int width, int height);

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



void CB_UpdateEyes(CubePoints *lefteye, CubePoints *righteye, bool stereo)
{
	_lefteye = lefteye;
	_righteye = righteye;
	_stereo = stereo;
}

void CB_InitReferences(bool &stereo, Shader *shader, ImageHandler *images, 
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

void CB_EnableVR(VRDevice *vr)
{
	_stereo = true;
	_usingVR = true;
	_vr = vr;
}

void CB_Init(STViewer *v, bool fullscreen)
{
	_viewer = v;

	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(_camera->Width, _camera->Height); // Defaults to 1280 x 800 windowed
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
	glutDisplayFunc(CB_Display);
	glutIdleFunc(_Idle);
	glutReshapeFunc(_Resize);
	glutSpecialFunc(Controls::ProcessGLUTKeys);
	glutKeyboardFunc(Controls::ProcessKeys);
	//glutMotionFunc(Controls::MouseMove); // This is super broken with 5-panel displays, just disable it.
	glutMouseWheelFunc(Controls::MouseWheel);
	//glutTimerFunc(5000, timerCleanup, 0);
}

void CB_InitMenus(std::vector<PanoInfo> &panoList)
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

void CB_Display()
{
	if (_usingVR) {
		glDisable(GL_CULL_FACE);
		for (unsigned int eyeIndex = 0; eyeIndex < 2; ++eyeIndex) {
			_shader->Bind();
			_shader->SetFloatUniform("TileWidth", _lefteye->m_TILEWIDTH);
			VRControllerStates controllers = getVRControllerState(_vr);

			bindEyeRenderSurface(_vr, eyeIndex);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			_images->BindTextures(*_shader, eyeIndex);

			glm::mat4x4 perspective = buildVRProjectionMatrix(_vr, eyeIndex);
			glm::mat4x4 view = glm::mat4_cast(glm::inverse(getVRHeadsetRotation(_vr)));
			// Correct right eye panorama alignment
			if (eyeIndex == 1)
			{
				float eyeRotation = -controllers.left.indexFingerTrigger * 0.2f;
				view = view * glm::eulerAngleYXZ(eyeRotation, 0.0f, 0.0f);
				printf("Rotation: %f\n", eyeRotation);
			}

			//if (eyeIndex == 1) printf("IPD: %fmm, Separation correction: %fmm\n", interpupillaryDistance*1000, separationCorrection*1000);
			// Todo: report camera yaw and pitch for people making annotations.
			float cameraYaw = 0;
			float cameraPitch = 0;
			//printf("Yaw: %f,\tPitch:%f\n", cameraYaw, cameraPitch);

			_shader->SetMatrixUniform("MVP", perspective*view);

			if (eyeIndex==0) {
				_lefteye->BindVAO();
				glDrawArrays(GL_POINTS, 0, _lefteye->m_NumVertices);
			}
			else {
				_righteye->BindVAO();
				glDrawArrays(GL_POINTS, 0, _righteye->m_NumVertices);
			}

			// Render objects in 3D space
			view = buildVRViewMatrix(_vr, eyeIndex, 0, 0, 0);
			view = glm::translate(view, getVRHeadsetPosition(_vr)); // Negate headset translation

			// Annotations
			_viewer->m_annotations.Display(perspective, view, eyeIndex, true);

			// GUI
			double uiDisplayWaitTime = 1.5;
			if (_globalTime - _viewer->m_lastUIInteractionTime < uiDisplayWaitTime) {
				float uiRadius = 0.65f;
				_viewer->m_gui.Display(getVRHeadsetRotation(_vr), perspective*view, uiRadius, _viewer->m_guiPanoSelection, true);
			}

			commitEyeRenderSurface(_vr, eyeIndex);
		}
		finishVRFrame(_vr);
		blitHeadsetView(_vr, 0);
	} 
	else
	{
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

		if (_viewer->m_displaygui) {
			// Center the GUI if we have multiple viewports
			_camera->SetViewport(_camera->LeftCameras[_camera->NumCameras / 2]);

			// Reset projection to "only one screen"
			glm::mat4 proj = glm::perspective(glm::radians(45.0f), 
				float(_camera->Width) / float(_camera->Height), 0.1f, 10000.0f);

			// Draw a new viewport to cover the whole scene
			glViewport(0, 0, _camera->Width, _camera->Height);

			_viewer->m_gui.Display(glm::quat(glm::inverse(_camera->View)),
				proj * _camera->View, _viewer->m_guiPanoSelection);

			//_viewer->m_gui.ShowCube(glm::inverse(_camera->View), proj * _camera->View, _globalTime);

			// Rebind main program
			glDisable(GL_CULL_FACE);
			_shader->Bind();
			_shader->SetFloatUniform("TileWidth", _lefteye->m_TILEWIDTH);
			_images->BindTextures(*_shader, 0);
			_lefteye->BindVAO();
			if (_stereo)
				_righteye->BindVAO();
		}

		glFlush();
	}

#ifdef DEBUG
	PRINT_GL_ERRORS
#endif
}

static void _Idle()
{
	LARGE_INTEGER time;
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&ticksPerSecond);
	float deltaTime = float(double(time.QuadPart - _programStartTime) / double(ticksPerSecond.QuadPart) - _globalTime);
	_globalTime = (time.QuadPart - _programStartTime) / double(ticksPerSecond.QuadPart);

	_viewer->Update(_globalTime, deltaTime);
}

static void _Resize(int w, int h)
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

static void _MainMenu(int choice)
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

static void _PanoMenu(int choice)
{
	_viewer->SelectPano(--choice);
}

static void _Cleanup()
{
	_viewer->Cleanup();
}