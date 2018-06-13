#include "Controls.h"

STViewer Controls::viewer;
double Controls::globalTime;
long long Controls::programStartTime;

// A great deal of this is just wrappers around Camera:: class calls

int Controls::DEBUG_row = 0;
int Controls::DEBUG_col = 0;
float Controls::DEBUG_camerastep = 1.0f;

void Controls::init(const char* panoFileAddress, bool stereo, bool fullscreen, bool fivepanel)
{
	int windowWidth = 1280;
	int windowHeight = 800;
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(windowWidth, windowHeight); // Defaults to 1280 x 800 windowed
	glutCreateWindow("TileViewer - ST Shader Annihilation Edition");
	if (fullscreen) {
		glutFullScreen();
	}

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	// Init STViewer
	viewer.Init(panoFileAddress, stereo, fivepanel, windowWidth, windowHeight);

	// Setup callbacks
	atexit(cleanup);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(resize);
	glutSpecialFunc(ProcessGLUTKeys);
	glutKeyboardFunc(ProcessKeys);
	//glutMotionFunc(Controls::MouseMove); // This is super broken with 5-panel displays, just disable it.
	glutMouseWheelFunc(MouseWheel);
	//glutTimerFunc(5000, timerCleanup, 0);

	// Init right-click menus
	std::vector<PanoInfo> panoList = viewer.getPanoList();
	int panomenu = glutCreateMenu(PanoMenu);
	for (unsigned int i = 0; i < panoList.size(); i++) {
		char buf[64];
		sprintf_s(buf, "%s", panoList[i].displayName.c_str());
		glutAddMenuEntry(buf, i + 1);
	}

	int mainmenu = glutCreateMenu(MainMenu);
	glutAddMenuEntry("Toggle ST scaling (F8)", 1);
	glutAddSubMenu("Pano Select", panomenu);
	glutAddMenuEntry("Next Pano (n)", 2);
	glutAddMenuEntry("Prev Pano (p)", 3);
	glutAddMenuEntry("Screenshot (F9)", 4);
	glutAddMenuEntry("Toggle Fullscreen (f)", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Init time
	LARGE_INTEGER time;
	QueryPerformanceCounter(&time);
	programStartTime = time.QuadPart;
	globalTime = time.QuadPart;

	// Pass control to GLUT and start main loop
	glutMainLoop();
}

void Controls::FlipDebug()
{
	//GLuint uDebug = glGetUniformLocation(program, "Debug");
	//glUniform1f(uDebug, DEBUG_FLAG);
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
	viewer.moveCamera(0, 0, FOVChange);
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
		viewer.reloadShaders();
		break;

	case GLUT_KEY_F4:
		//Camera::UpdateCameras();
		break;

	case GLUT_KEY_F5:
		//fprintf(stderr, "FOV is at %f\n", Camera::FOV);
#ifdef DEBUG
		viewer.WaitingThreads();
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
		//ImageHandler::WindowDump(Camera::Width, Camera::Height);
		break;
#ifdef DEBUG
	case GLUT_KEY_F10:
		viewer.PrintAverage();
		break;
#endif
	}
	
	viewer.moveCamera(pitchChange, yawChange, FOVChange);
}

void Controls::ProcessKeys(unsigned char key, int x, int y)
{
	float average = 0.0f;
	switch (key) {
	case '1':
	case '2':
		viewer.reloadShaders();

#ifdef DEBUG
	case '3':
		viewer.RebindVAO();
		break;
#endif
	case 'f':
		glutFullScreenToggle();
		break;

	case 'h':
		viewer.ToggleStereo();
		break;

	case 'n':
		viewer.NextPano();
		break;

	case 'p':
		viewer.PrevPano();
		break;

	case 'r':
		/*Camera::FOV = DEBUG_fov;
		Camera::Pitch = 0.0f;
		Camera::UpdateMVP();
		Camera::UpdateCameras();*/
		break;

	case 'R':
		viewer.ReloadPano();
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
		viewer.NextPano();
		break;

	case 3:
		viewer.PrevPano();
		break;

	case 4:
		//ImageHandler::WindowDump(Camera::Width, Camera::Height);
		break;

	case 5:
		glutFullScreenToggle();
		break;
	}
}

void Controls::PanoMenu(int choice)
{
	viewer.SelectPano(--choice);
}

void Controls::cleanup() {
	viewer.cleanup();
}

void Controls::display() {
	viewer.display(globalTime);
}

void Controls::idle() {
	// Update time
	LARGE_INTEGER time;
	LARGE_INTEGER ticksPerSecond;
	QueryPerformanceCounter(&time);
	QueryPerformanceFrequency(&ticksPerSecond);
	float deltaTime = float(time.QuadPart-programStartTime)/float(ticksPerSecond.QuadPart) - globalTime;
	globalTime = (time.QuadPart-programStartTime) / double(ticksPerSecond.QuadPart);
	
	viewer.update(globalTime, deltaTime);
}

void Controls::resize(int w, int h) {
	viewer.resize(w, h);
}