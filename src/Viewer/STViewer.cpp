// STViewer.cpp : Runs the TileViewer using ST quad manipulations for texture loading
//

#include "STViewer.h"
#include "STViewerCallbacks.h"


#ifdef DEBUG
#define TIMERSTART t1 = std::chrono::high_resolution_clock::now();
#define TIMERSTOP std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count()
#define DURATION std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
#endif


/*---------------- Public Functions ----------------*/

STViewer::STViewer(const char* panoURI, bool stereo, bool fivepanel, 
	bool fullscreen, int viewWidth, int viewHeight) :
	m_stereo(stereo),
	m_fivepanel(fivepanel),
	m_fullscreen(fullscreen)
{
	m_images.InitPanoList(panoURI);

	// Init camera
	if (m_fivepanel)
		m_camera.Init(5, viewWidth, viewHeight);
	else
		m_camera.Init(1, viewWidth, viewHeight);
	if (m_stereo)
		m_camera.SplitHorizontal();

	downloadPool = new Threads::ThreadPool(std::thread::hardware_concurrency());
	texturePool = new Threads::ThreadPool(std::thread::hardware_concurrency()-1);
	workerPool = new Threads::ThreadPool(2);

	m_panolist = m_images.m_panoList;
	m_currentPano = 0;

	m_maxDepth = 3; // Magic hardcoded number (powers of 2);

	m_LoadedTextures = new ImageQueue();

	initGL();
	initVR();
	initTextures();

	glutMainLoop();
}

void STViewer::ToggleStereo()
{
	m_stereo = !m_stereo;
	m_camera.SplitHorizontal();
	m_images.InitStereo();
	if (m_stereo) {
		if (m_RightEye == NULL) {
			m_RightEye = new CubePoints(m_maxDepth, 1);
			_UpdateEyes(m_LeftEye, m_RightEye);
		}
		else {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
		m_camera.UpdateCameras();
		workerPool->submit([](STViewer* v) { v->loadAllQuadDepths(); }, this);
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
		m_images.m_currentPano = m_currentPano;
		resetImages();
	}
	else {
		fprintf(stderr, "Invalid Pano Selection\n");
	}
}

void STViewer::ReloadShaders()
{
	m_shader.Reload();
	m_images.BindTextures(m_shader, 0);
	m_images.BindTextures(m_shader, 1);
	m_shader.Bind();
}

void STViewer::MoveCamera(float pitchChange, float yawChange, float FOVChange)
{
	m_camera.FOV += FOVChange;
	if (m_camera.FOV < 0) {
		m_camera.FOV = 1.0f;
	}
	m_camera.Pitch += pitchChange;
	m_camera.Yaw += yawChange;

	m_camera.UpdateMVP();
	m_camera.UpdateCameras();
}

void STViewer::ResetCamera()
{
	m_camera.FOV = m_camera.ResetFOV;
	m_camera.Pitch = 0.0f;
	m_camera.UpdateMVP();
	m_camera.UpdateCameras();
}

std::vector<PanoInfo> STViewer::GetPanos()
{
	return m_panolist;
}

void STViewer::FlipDebug()
{
	m_shader.FlipDebug();
}

/*---------------- Private Functions ----------------*/

//This ends up being slower than LoadAllQuadDepths, probably from fewer threads running?
void STViewer::loadAllFaceDepths()
{
	int eyes = 1;
	if (m_stereo)
		eyes = 2;

	ImageHandler& images = m_images;
	for (int depth = 0; depth < 4; depth++) {
		for (int face = 0; face < 6; face++) {
			for (int eye = 0; eye < eyes; eye++) {
				texturePool->submit([&images](int face, int depth, int eye)
				{
					images.LoadFaceImage(face, depth, eye);
				}, face, depth, eye);
			}
		}
	}
}

void STViewer::loadAllQuadDepths()
{
	ImageHandler& images = m_images;
	for (int i = 0; i < downloadPool->size(); i++)
		downloadPool->submit([&images]() { images.LoadQuadImage(); });
	for (int i = 0; i < texturePool->size(); i++) {
		texturePool->submit([&images]() { images.Decompress(); });
	}
}

void STViewer::Update(void)
{
	if (m_usingVR) {
		updateVRDevice(&m_vr);
		VRControllerStates controllers = getVRControllerState(&m_vr);
		if (controllers.right.button1.pressed) {
			NextPano();
		}
		if (controllers.right.button2.pressed) {
			PrevPano();
		}
	}

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
		m_images.LoadImageData(image);
	}

	// Check to see if any thread has updated cube quad depths, if so rebind VAO/VBO data
	while (m_LeftEye->Ready())
		m_LeftEye->RebindVAO();
	while (m_stereo && m_RightEye->Ready())
		m_RightEye->RebindVAO();


	_Display();
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
	m_images.ClearQueues();

	// Wait for any threads that are mid-work to finish
	while (!texturePool->allstopped());
	while (!downloadPool->allstopped());

	/* Turn off queue discarding */
	m_LoadedTextures->ToggleDiscard();

	m_images.InitURLs(m_currentPano, m_stereo);

	/* _NOW_ queue up all the requests for the new panorama */
	workerPool->submit([](STViewer* v) {
		v->loadAllQuadDepths();
	}, this);
	
	
	
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
}

void STViewer::resetCubes()
{
	// Out of order logic so we can toggle Stereo on during runtime and properly generate the RightEye
	if (m_stereo && m_RightEye == NULL)
		m_RightEye = new CubePoints(m_maxDepth, 1);

	// LeftEye is default
	if (m_LeftEye == NULL)
		m_LeftEye = new CubePoints(m_maxDepth, 0);
	else {
		m_LeftEye->ResetDepth();
		m_LeftEye->RebindVAO();
		if (m_stereo) {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
	}
	
	_UpdateEyes(m_LeftEye, m_RightEye);

	// Wait for Level 0 to be loaded
	if (m_panolist.size() > 0)
		while (m_LoadedTextures->Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}

void STViewer::initGL()
{
	_InitCallbacks(this, m_fullscreen);
	_InitReferences(m_stereo, &m_shader, &m_images, m_LeftEye, m_RightEye, &m_camera);
	_InitMenus(m_panolist);

	m_shader.CreateProgram("Shader.geom", "Shader.vert", "Shader.frag");
	m_shader.Bind();

}

void STViewer::initVR()
{
	m_usingVR = createVRDevice(&m_vr, m_camera.Width, m_camera.Height);
	if (m_usingVR) {
		m_stereo = true;
		updateVRDevice(&m_vr);
		_EnableVR(&m_vr);
	}
}

void STViewer::initTextures()
{
	m_images.InitTextureAtlas(m_stereo, m_LoadedTextures);

	// Trigger our logic pattern to init cubes and start loading images
	resetImages();

	m_pointCount = m_LeftEye->m_NumVertices;
	m_images.BindTextures(m_shader, 0);

	fprintf(stderr, "ST Viewer Initialized\n");
}

void STViewer::Cleanup()
{
	downloadPool->stopall();
	texturePool->stopall();
	workerPool->stopall();
	m_LoadedTextures->Clear();
	m_images.ClearQueues();
	while (!texturePool->allstopped());
	m_images.ClearQueues(); // Sanity check
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