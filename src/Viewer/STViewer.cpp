// STViewer.cpp : Runs the TileViewer using ST quad manipulations for texture loading
//

#include "STViewer.h"

#include "GLhandles.h"
#include "ShaderHelper.h"
#include "ImageHandler.h"
#include "ImageQueue.h"

#include "Camera.h"
#include "Controls.h"

#ifdef DEBUG
#define TIMERSTART t1 = std::chrono::high_resolution_clock::now();
#define TIMERSTOP std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count()
#define DURATION std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
#endif

// GL handles
GLuint program;
GLuint VBO;
GLuint VAO;
GLsizei pointCount;


// What the hell is the point of a header file if we have to redeclare static references in the cpp file?
CubePoints *STViewer::m_LeftEye;
CubePoints *STViewer::m_RightEye;
std::vector<PanoInfo> STViewer::m_panolist;
unsigned int STViewer::m_currentPano;

Threads::ThreadPool *STViewer::downloadPool;
Threads::ThreadPool *STViewer::texturePool;
Threads::ThreadPool *STViewer::workerPool;

int STViewer::m_maxDepth;

ImageQueue *STViewer::m_LoadedTextures;

#ifdef USE_VR
// VR Data
VRDevice vrDevice;
bool STViewer::m_usingVR = false;
#endif
bool STViewer::imagesNeedResetting = false;

#ifdef DEBUG
int STViewer::m_facecount[2][6];
std::vector<float> STViewer::m_average;
std::chrono::high_resolution_clock::time_point STViewer::t1;
#endif

/*---------------- Public Functions ----------------*/

#ifdef USE_VR
void STViewer::Init(VRDevice &vrRef)
#else
void STViewer::Init()
#endif
{

	// RAII keeps calling destructors immediately if we try to call
	// texturePool = &Threads::ThreadPool(numthreads)
	// So whatever, do this insanely stupid thing
	Threads::ThreadPool poolref_1(std::thread::hardware_concurrency());
	Threads::ThreadPool poolref_2(std::thread::hardware_concurrency()-1);
	Threads::ThreadPool poolref_3(2);
	downloadPool = &poolref_1;
	texturePool = &poolref_2;
	workerPool = &poolref_3;

	m_panolist = ImageHandler::m_panoList;
	m_currentPano = 0;

	m_maxDepth = 3; // Magic hardcoded number (powers of 2);

	m_LoadedTextures = new ImageQueue();

	atexit(cleanup);
	initWindowAndGL();

#ifdef USE_VR
	// Attempt to create a VR device and enable it as appropriate
	vrDevice = vrRef;
	m_usingVR = createVRDevice(&vrDevice, Camera::Width, Camera::Height);
	if (m_usingVR) {
		stereo = true;
		updateVRDevice(&vrDevice);
	}
#endif

	initTextures();
	glutMainLoop();
}

void STViewer::ToggleStereo()
{
	stereo = !stereo;
	Camera::SplitHorizontal();
	ImageHandler::InitStereo(program);
	if (stereo) {
		if (m_RightEye == NULL) {
			m_RightEye = new CubePoints(m_maxDepth, 1);
		}
		else {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
		Camera::UpdateCameras();
		workerPool->submit([]() { loadAllQuadDepths(); });
	}
}

void STViewer::ReloadPano()
{
	resetImages();
}

void STViewer::PrevPano()
{
	if (m_currentPano > 0) {
		m_currentPano--;
		SelectPano(m_currentPano);
	}
	else {
		fprintf(stderr, "At first pano\n");
	}
}

void STViewer::NextPano()
{
	if (m_currentPano < m_panolist.size() - 1) {
		m_currentPano++;
		SelectPano(m_currentPano);
	}
	else {
		fprintf(stderr, "At last pano\n");
	}
}

void STViewer::SelectPano(int pano)
{
	if (pano > -1 && pano < m_panolist.size()) {
		m_currentPano = pano;
		ImageHandler::m_currentPano = m_currentPano;
		resetImages();
	}
	else {
		fprintf(stderr, "Invalid Pano Selection\n");
	}
}

/*---------------- Private Functions ----------------*/

void STViewer::display()
{
#ifdef USE_VR
	if (m_usingVR) {
		for (unsigned int eyeIndex = 0; eyeIndex < 2; ++eyeIndex) {
			bindEyeRenderSurface(&vrDevice, eyeIndex);
			// The worlds most basic draw routine!
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			if (eyeIndex == 0) {
				glBindVertexArray(m_LeftEye->m_PositionVAOID);
			}
			else {
				glBindVertexArray(m_RightEye->m_PositionVAOID);
			}

			glDisable(GL_CULL_FACE);
			ImageHandler::RebindTextures(program, eyeIndex);

			OVR::Matrix4f perspective = buildVRProjectionMatrix(&vrDevice, eyeIndex);
			OVR::Matrix4f view = buildVRViewMatrix(&vrDevice, eyeIndex, 0, 0, 0) * OVR::Matrix4f::Translation(getVRHeadsetPosition(&vrDevice));
			GLuint MatrixID = glGetUniformLocation(program, "MVP");
			glUniformMatrix4fv(MatrixID, 1, GL_TRUE, (float*)&(perspective*view));

			glDrawArrays(GL_POINTS, 0, pointCount);
			commitEyeRenderSurface(&vrDevice, eyeIndex);
		}
		finishVRFrame(&vrDevice);
		blitHeadsetView(&vrDevice, 0);
	}
	else {
#endif
		// The worlds most basic draw routine!
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	
		glBindVertexArray(m_LeftEye->m_PositionVAOID);
		for (int i = 0; i < Camera::NumCameras; i++) {
			Camera::SetViewport(Camera::LeftCameras[i]);
		}

		if (stereo) {
			glBindVertexArray(m_RightEye->m_PositionVAOID);
			ImageHandler::RebindTextures(program, 1);
			for (int i = 0; i < Camera::NumCameras; i++) {
				Camera::SetViewport(Camera::RightCameras[i]);
			}
			ImageHandler::RebindTextures(program, 0);
		}

		glFlush();
#ifdef USE_VR
	}
#endif
}

//This ends up being slower than LoadAllQuadDepths, probably from fewer threads running?
void STViewer::loadAllFaceDepths()
{
	int eyes = 1;
	if (stereo)
		eyes = 2;

	for (int depth = 0; depth < 4; depth++) {
		for (int face = 0; face < 6; face++) {
			for (int eye = 0; eye < eyes; eye++) {
				texturePool->submit([](int face, int depth, int eye)
				{
					ImageHandler::LoadFaceImage(face, depth, eye);
					//ImageHandler::LoadChunked();
				}, face, depth, eye);
			}
		}
	}
}

void STViewer::loadAllQuadDepths()
{
	for (int i = 0; i < downloadPool->size(); i++)
		downloadPool->submit([]() { ImageHandler::LoadQuadImage(); });
	for (int i = 0; i < texturePool->size(); i++) {
		//texturePool->submit([]() { ImageHandler::LoadQuadImage(); });
		texturePool->submit([]() { ImageHandler::Decompress(); });
	}
}

void STViewer::idleFunc(void)
{
#ifdef USE_VR
	if (m_usingVR) {
		updateVRDevice(&vrDevice);
		VRControllerStates controllers = getVRControllerState(&vrDevice);
		if (controllers.right.button1.pressed) {
			NextPano();
		}
		if (controllers.right.button2.pressed) {
			PrevPano();
		}
	}
#endif
	// 16.67ms per frame, takes us ~0.5ms to send an image to the GPU/Update quad depth (async GL calls)
	// Queue gets emptied fast enough that we never actually load 32 images sequentially, but we can do
	// Up to that many without skipping a frame.
	for (int i = 0; !m_LoadedTextures->IsEmpty() && i < 32; i++) {
		ImageData *image = m_LoadedTextures->Dequeue();

		// TODO: We can skip storing these on the stack if we convert to using std::share_ptr
		int face = image->face;
		int row = image->row;
		int col = image->col;
		int depth = image->depth;
		int eye = image->eye;
#ifdef DEBUG
		m_facecount[eye][face]++;
		if (depth == 3 && m_facecount[eye][face] == 85) {
			long long now = NOW;
			fprintf(stderr, "Face %d Eye %d finished in %lld ms!\n", face, eye, now);
			m_average.push_back(now);
		}
#endif

		if (eye == 0) {
			workerPool->submit([&](int face, int row, int col, int depth)
			{
				m_LeftEye->QuadSetDepth(face, row, col, depth);
			}, face, row, col, depth);
		}
		else {
			workerPool->submit([&](int face, int row, int col, int depth)
			{
				m_RightEye->QuadSetDepth(face, row, col, depth);
			}, face, row, col, depth);
		}
		ImageHandler::LoadImageData(image);
	}

	// Check to see if any thread has updated cube quad depths, if so rebind VAO/VBO data
	while (m_LeftEye->Ready())
		m_LeftEye->RebindVAO();
	while (stereo && m_RightEye->Ready())
		m_RightEye->RebindVAO();


	display();
	glutSwapBuffers();
}

void STViewer::resetImages()
{
	/* 
		The ordering of this is important, if we reset our quad depths and render even a single frame
		before we've loaded in the next panoramas Level 0 textures, we get this huge zoom effect on one
		tile from the last panorama, and it looks terrible.
	*/
	downloadPool->stopall();
	texturePool->stopall();
	workerPool->stopall();

	// Clear out everything that's still sitting in our queue and enable discarding of new items
	m_LoadedTextures->Clear();
	ImageHandler::ClearQueues();

	// Wait for any threads that are mid-work to finish
	while (!texturePool->allstopped());
	while (!downloadPool->allstopped());

	/* Sanity Check */
	ImageHandler::ClearQueues();

	/* Turn off queue discarding */
	m_LoadedTextures->ToggleDiscard();

	ImageHandler::InitURLs(m_currentPano, stereo);
	
	/* _NOW_ queue up all the requests for the new panorama */
	workerPool->submit([]() {
		loadAllQuadDepths();
	});
	
	
	
#ifdef DEBUG
	// These aren't really important anymore but might be useful for debugging
	// We can spare the 0.01ms it takes to reset them
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			m_facecount[i][j] = 0;
		}
	}
#endif

	// Finally, reset our quad depths on our cube, as we have a new pano to display
	resetCubes();
	imagesNeedResetting = false;
}

void STViewer::resetCubes()
{
	// Out of order logic so we can toggle Stereo on during runtime and properly generate the RightEye
	if (stereo && m_RightEye == NULL)
		m_RightEye = new CubePoints(m_maxDepth, 1);

	// LeftEye is default
	if (m_LeftEye == NULL)
		m_LeftEye = new CubePoints(m_maxDepth, 0);
	else {
		m_LeftEye->ResetDepth();
		m_LeftEye->RebindVAO();
		if (stereo) {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
	}
	
	// Wait for Level 0 to be loaded
	if (m_panolist.size() > 0)
		while (m_LoadedTextures->Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}

void STViewer::resizeFunc(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	Camera::Width = w;
	Camera::Height = h;
	fprintf(stderr, "%d %d\n", Camera::Width, Camera::Height);
	Camera::UpdateMVP();
	Camera::UpdateCameras();
#ifdef USE_VR
	if (m_usingVR) {
		resizeMirrorTexture(&vrDevice, w, h);
	}
#endif
}

void STViewer::initWindowAndGL()
{
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

	program = ShaderHelper::CreateProgram();
}

void STViewer::initTextures()
{
	ImageHandler::InitTextureAtlas(program, stereo, m_LoadedTextures);

	// Trigger our logic pattern to init cubes and start loading images
	resetImages();

	pointCount = m_LeftEye->m_NumVertices;

	GLuint uTileWidth = glGetUniformLocation(program, "TileWidth");
	glUniform1f(uTileWidth, m_LeftEye->m_TILEWIDTH);

	if (fivepanel)
		Camera::Init(5);
	else
		Camera::Init(1);
	if (stereo)
		Camera::SplitHorizontal();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	fprintf(stderr, "ST Viewer Initialized\n");
}

void STViewer::initCallbacks()
{
	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutReshapeFunc(resizeFunc);
	glutSpecialFunc(Controls::ProcessGLUTKeys);
	glutKeyboardFunc(Controls::ProcessKeys);
	//glutMotionFunc(Controls::MouseMove); // This is super broken with 5-panel displays, just disable it.
	glutMouseWheelFunc(Controls::MouseWheel);
	//glutTimerFunc(5000, timerCleanup, 0);
}

void STViewer::initMenus()
{
	/*  Build some nice menus to use  */
	int panomenu = glutCreateMenu(Controls::PanoMenu);
	for (unsigned int i = 0; i < m_panolist.size(); i++) {
		char buf[64];
		sprintf_s(buf, "%s", m_panolist[i].displayName.c_str());
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

void STViewer::cleanup()
{
	texturePool->stopall();
	workerPool->stopall();
	m_LoadedTextures->Clear();
	while (!texturePool->allstopped());
}

#ifdef DEBUG

void STViewer::PrintAverage()
{
	float total = 0.0f;
	for (auto time : m_average)
		total += time;
	fprintf(stderr, "Current average time to load a Pano in full: %f\n", total / m_average.size());
}

void STViewer::WaitingThreads()
{
	fprintf(stderr, "Number of download threads: %d\n", downloadPool->running());
	fprintf(stderr, "Number of texture threads: %d\n", texturePool->running());
}

void STViewer::RebindVAO()
{
	m_LeftEye->RebindVAO();
}

#endif