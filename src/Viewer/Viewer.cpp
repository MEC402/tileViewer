// Viewer.cpp : Defines the entry point for the console application.
//

// I've made a terrible mess of this file.  Again.

#include "Viewer.h"

#include "GLhandles.h"
#include "ShaderHelper.h"
#include "ImageHandler.h"
#include "ImageQueue.h"
#include "VR.h"

#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"
#include "Camera.h"
#include "Controls.h"

#define DEBUG

#ifdef DEBUG
#define TIMERSTART t1 = std::chrono::high_resolution_clock::now();
#define TIMERSTOP std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count()
#define DURATION std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
#endif

// Flags
bool fullscreen = false;
bool stereo = false;
bool fivepanel = false;

// GL handles
GLuint program;
GLuint VBO;
GLuint VAO;
GLsizei pointCount;

// Geometry data
CubePoints *LeftEye;
CubePoints *RightEye;

// Depth for number of cube generation ///////
int maxImageDepth = 3;

// VR Data
VRDevice vrDevice;
bool usingVR;

// Panolist
std::vector<PanoInfo> panolist;
int currentPanoIndex = 0;

// Thread pool /////////////////////////////
Threads::ThreadPool tpool_(std::thread::hardware_concurrency() - 1); // Use everything, whatever
Threads::ThreadPool tpool2_(1);
std::deque<Threads::ThreadPool::TaskFuture<void>> v; // So we can collect waiting promises from the threadpool
std::deque<Threads::ThreadPool::TaskFuture<void>> v2;

bool DEBUG_FLAG = false;

#ifdef DEBUG
// Local face data
int facedepths[2][6];
int facecount[2][6];

// Timer values
std::chrono::high_resolution_clock::time_point timerStart;
std::chrono::high_resolution_clock::time_point t1;
long long timerEnd[2][6];
#endif

// State flag for poolhandler() //////////
bool handling1 = false;
bool handling2 = false;

void timerFunc(int value)
{		
	if (!v.empty() && !handling1) {
		handling1 = true;
		std::thread t1([]() {
			while (!v.empty()) {
				try {
					v.front().get();
				}
				catch (std::future_error& e) {
#ifdef DEBUG
					fprintf(stderr, "Caught a future error: %s\n", e.what());
#endif
				}
				v.pop_front();
			}
			handling1 = false;
		});
		t1.detach();
	}
	if (!v2.empty() && !handling2) {
		handling2 = true;
		std::thread t2([]() {
			while (!v2.empty()) {
				try {
					v2.front().get();
				}
				catch (std::future_error& e) {
#ifdef DEBUG
					fprintf(stderr, "Caught a future error: %s\n", e.what());
#endif
				}
				v2.pop_front();
			}
			handling2 = false;
		});
		t2.detach();
	}
	glutTimerFunc(5000, timerFunc, 0);
}

void display()
{
	if (usingVR) {
		for (unsigned int i = 0; i < 2; ++i) {
			bindEyeRenderSurface(&vrDevice, i);
			// The worlds most basic draw routine!
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glBindVertexArray(VAO);

			OVR::Matrix4f perspective = buildVRProjectionMatrix(&vrDevice, i);
			OVR::Matrix4f view = buildVRViewMatrix(&vrDevice, i, 0, 0, 0);
			view.SetTranslation(OVR::Vector3f(0, 0, 0));
			GLuint MatrixID = glGetUniformLocation(program, "MVP");
			glUniformMatrix4fv(MatrixID, 1, GL_TRUE, (float*)&(perspective*view));

			glDrawArrays(GL_POINTS, 0, pointCount);
			commitEyeRenderSurface(&vrDevice, i);
		}
		finishVRFrame(&vrDevice);
		blitHeadsetView(&vrDevice, 0);
	}
	else {
		// The worlds most basic draw routine!
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	
		glBindVertexArray(LeftEye->m_PositionVAOID);
		for (int i = 0; i < Camera::NumCameras; i++) {
			Camera::SetViewport(Camera::LeftCameras[i]);
		}

		if (stereo) {
			glBindVertexArray(RightEye->m_PositionVAOID);
			ImageHandler::RebindTextures(program, 1);
			for (int i = 0; i < Camera::NumCameras; i++) {
				Camera::SetViewport(Camera::RightCameras[i]);
			}
			ImageHandler::RebindTextures(program, 0);
		}

		glFlush();
	}
}

//This ends up being slower than LoadAllQuadDepths, probably from fewer threads running?
void LoadAllFaceDepths()
{
	int eyes = 1;
	if (stereo)
		eyes = 2;
	
	for (int depth = 0; depth < 4; depth++) {
		for (int face = 0; face < 6; face++) {
			for (int eye = 0; eye < eyes; eye++) {
				v.emplace_back(tpool_.submit([](int face, int depth, int eye)
				{
					ImageHandler::LoadFaceImage(face, depth, eye);
				}, face, depth, eye));
			}
		}
	}
}

void LoadAllQuadDepths(int eye)
{
	// WEW
	for (int depth = 0; depth < 4; depth++) {	
		int stepping = 8 / (int)pow(2, depth);
		for (int row = 0; row < 8; row += stepping) {
			for (int col = 7; col >= 0; col -= stepping) {
				for (int face = 0; face < 6; face++) {
					v.emplace_back(tpool_.submit([](int face, int row, int col, int depth, int eye)
					{
						ImageHandler::LoadQuadImage(face, row, col, depth, eye);
					}, face, row, col, depth, eye));
				}
			}
		}
	}
}

void idleFunc(void)
{
	if (usingVR) {
		updateVRDevice(&vrDevice);
	}

	for (int i = 0; i < 32; i++) {
		if (!ImageQueue::IsEmpty()) {
			ImageData *image = ImageQueue::Dequeue();

			// TODO: We can skip storing these on the stack if we convert to using std::share_ptr
			int face = image->face;
			int row = image->row;
			int col = image->col;
			int depth = image->depth;
			int eye = image->eye;

#ifdef DEBUG
			facecount[eye][face]++;
			if (depth == 3 && facecount[eye][face] == 85)
				fprintf(stderr, "Face %d Eye %d finished in %lld ms!\n", face, eye, NOW);
#endif

			if (eye == 0) {
				v2.emplace_back(tpool2_.submit([](int face, int row, int col, int depth)
				{
					LeftEye->QuadSetDepth(face, row, col, depth);
				}, face, row, col, depth));
			}
			else {
				v2.emplace_back(tpool2_.submit([](int face, int row, int col, int depth)
				{
					RightEye->QuadSetDepth(face, row, col, depth);
				}, face, row, col, depth));
			}
			ImageHandler::LoadImageData(image);
		}
		else {
			break;
		}
	}

	// Check to see if any thread has updated cube quad depths, if so rebind VAO/VBO data
	if (LeftEye->Ready)
		LeftEye->RebindVAO();
	if (stereo && RightEye->Ready)
		RightEye->RebindVAO();


	display();
	glutSwapBuffers();
}

void ToggleStereo()
{
	bool last = stereo;
	stereo = !stereo;
	Camera::SplitHorizontal();
	ImageHandler::InitStereo(program);
	if (!last) {
		if (RightEye == NULL) {
			RightEye = new CubePoints(maxImageDepth, 1);
		}
		else {
			RightEye->ResetDepth();
			RightEye->RebindVAO();
		}
		Camera::UpdateCameras();
		std::thread t(LoadAllQuadDepths, 1);
		t.detach();
	}
}

void ReloadPano()
{
	resetImages();
}

void PrevPano()
{
	if (currentPanoIndex > 0) {
		currentPanoIndex--;
		SelectPano(currentPanoIndex);
	}
	else {
		fprintf(stderr, "At first pano\n");
	}
}

void NextPano()
{
	if (currentPanoIndex < panolist.size() - 1) {
		currentPanoIndex++;
		SelectPano(currentPanoIndex);
	}
	else {
		fprintf(stderr, "At last pano\n");
	}
}

void SelectPano(int pano)
{
	if (pano > -1 && pano < panolist.size()) {
		currentPanoIndex = pano;
		ImageHandler::m_currentPano = currentPanoIndex;
		resetImages();
	}
	else {
		fprintf(stderr, "Invalid Pano Selection\n");
	}
}

void resetImages()
{
	/* 
		The ordering of this is important, if we reset our quad depths and render even a single frame
		before we've loaded in the next panoramas Level 0 textures, we get this huge zoom effect on one
		tile from the last panorama, and it looks terrible.
	*/

	/* First, stop all pending requests in our ThreadPool queue(s) */
	tpool_.stopall();
	tpool2_.stopall();

	/* Next, clear out everything that's still sitting in our queue and enable discarding of new items */
	ImageQueue::Clear();

	/* Wait for all executing threads to finish loading things (and discarding them) */
	while (!tpool_.allstopped());
	/* Turn off queue discarding */
	ImageQueue::ToggleDiscard();

	/* _NOW_ queue up all the requests for the new panorama */
	std::thread t(LoadAllQuadDepths, 0);
	t.detach();
	if (stereo) {
		std::thread t2(LoadAllQuadDepths, 1);
		t2.detach();
	}

	
#ifdef DEBUG
	// These aren't really important anymore but might be useful for debugging
	// We can spare the 0.01ms it takes to reset them
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			facedepths[i][j] = 1;
			facecount[i][j] = 0;
		}
	}
#endif

	// Finally, reset our quad depths on our cube, as we have a new pano to display
	resetCubes();
}

void resetCubes()
{
	// Out of order logic so we can toggle Stereo on during runtime and properly generate the RightEye
	if (stereo && RightEye == NULL)
		RightEye = new CubePoints(maxImageDepth, 1);

	// LeftEye is default
	if (LeftEye == NULL)
		LeftEye = new CubePoints(maxImageDepth, 0);
	else {
		LeftEye->ResetDepth();
		LeftEye->RebindVAO();
		if (stereo) {
			RightEye->ResetDepth();
			RightEye->RebindVAO();
		}
	}
	
	// Wait for Level 0 to be loaded
	while (ImageQueue::Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}

void resizeFunc(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	Camera::Width = w;
	Camera::Height = h;
	fprintf(stderr, "%d %d\n", Camera::Width, Camera::Height);
	Camera::UpdateMVP();
	Camera::UpdateCameras();
}

void initCallbacks()
{
	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutReshapeFunc(resizeFunc);
	glutSpecialFunc(Controls::ProcessGLUTKeys);
	glutKeyboardFunc(Controls::ProcessKeys);
	//glutMotionFunc(Controls::MouseMove); // This is super broken with 5-panel displays, just disable it.
	glutMouseWheelFunc(Controls::MouseWheel);
	glutTimerFunc(3000, timerFunc, 1);
}

void initMenus()
{
	/*  Build some nice menus to use  */
	int panomenu = glutCreateMenu(Controls::PanoMenu);
	for (int i = 0; i < panolist.size(); i++) {
		char buf[64];
		sprintf_s(buf, "%s", panolist[i].displayName.c_str());
		glutAddMenuEntry(buf, i + 1);
	}

	int mainmenu = glutCreateMenu(Controls::MainMenu);
	glutAddMenuEntry("Toggle ST scaling (F8)", 1);
	glutAddSubMenu("Pano Select", panomenu);
	glutAddMenuEntry("Next Pano (n)", 2);
	glutAddMenuEntry("Prev Pano (p)", 3);
	glutAddMenuEntry("Screenshot (F9)", 4);
	glutAddMenuEntry("Toggle Fullscreen (f)", 5);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void cleanup()
{
	tpool_.stopall();
	tpool2_.stopall();
	ImageQueue::Clear();
	while (!tpool_.allstopped());
}

int main(int argc, char **argv)
{
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	for (int i = 0; i < argc; i++) {
		if (argv[i] == std::string("-f"))
			fullscreen = true;

		if (argv[i] == std::string("-s"))
			stereo = true;

		if (argv[i] == std::string("-5"))
			fivepanel = true;
	}

	ImageHandler::InitPanoListFromOnlineFile(argv[argc-1]);
	panolist = ImageHandler::m_panoList;

	atexit(cleanup);

	/* setup the size, position, and display mode for new windows */
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);
 	glutInitWindowSize(Camera::Width, Camera::Height); // Defaults to 1280 x 800 windowed
	glutCreateWindow("TileViewer - ST Shader Annihilation Edition");
	if (fullscreen)
		glutFullScreen();

	initCallbacks();
	initMenus();

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Attempt to create a VR device and enable it as appropriate
	usingVR = createVRDevice(&vrDevice, Camera::Width, Camera::Height);
	if (usingVR) {
		updateVRDevice(&vrDevice);
	}

	// Create GL program
	program = ShaderHelper::CreateProgram();
	// Initialize our texture Atlas's
	// TODO: We need to be able to generate the second eye's atlases on demand if we switch to stereo mode at runtime
	ImageHandler::InitTextureAtlas(program, stereo);

	// Trigger our "Reset all images, cube depths, and load the current panoramas data set" functions
	resetImages();
	
	// Should be the same for both eyes, just pick Left by default
	pointCount = LeftEye->m_NumVertices; 

	GLuint uTileWidth = glGetUniformLocation(program, "TileWidth");
	glUniform1f(uTileWidth, LeftEye->m_TILEWIDTH);
	

	// Initialize Camera's
	if (fivepanel)
		Camera::Init(5);
	else
		Camera::Init(1);

	// If we're running with -s then split cameras now
	if (stereo)
		Camera::SplitHorizontal();

	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	fprintf(stderr, "You should be seeing something on the screen now.\n");

	// Debug timer, not really useful here
	//timerStart = std::chrono::high_resolution_clock::now();
	//LoadStaticFaces();
	glutMainLoop();
}