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
	bool fullscreen, int viewWidth, int viewHeight, RemoteClient *remote) :
	m_stereo(stereo),
	m_fivepanel(fivepanel),
	m_fullscreen(fullscreen),
	m_remote(remote)
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

	m_LoadedTextures = new SafeQueue<ImageData*>();

	initGL();
	initVR();
	initTextures();

	m_gui.Create(m_panolist);
	m_lastUIInteractionTime = 0;
	m_guiPanoSelection = 0;
	m_displaygui = !m_usingVR;
	m_linear = true;

	Controls::SetViewer(this);

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
		}
		else {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
		m_camera.UpdateCameras();
		workerPool->submit([](STViewer* v) { v->loadAllQuadDepths(); }, this);
	}
	CB_UpdateEyes(m_LeftEye, m_RightEye, m_stereo);
}


/* ---------------------- Panorama / Texture Controls ---------------------- */

void STViewer::ReloadPano()
{
	resetImages();
}

void STViewer::PrevPano()
{
	if (--m_currentPano < 0)
		m_currentPano = m_panolist.size() - 1;
	SelectPano(m_currentPano);
}

void STViewer::NextPano()
{
	if (++m_currentPano >= m_panolist.size())
		m_currentPano = 0;
	SelectPano(m_currentPano);
}

void STViewer::SelectPano(int pano)
{
	if (pano > -1 && pano < m_panolist.size()) {
		if (m_displaygui) {
			m_guiPanoSelection = pano;
		}
		else {
			m_currentPano = pano;
			m_images.m_currentPano = m_currentPano;
			resetImages();
		}
	}
	else {
		fprintf(stderr, "Invalid Pano Selection\n");
	}
}

void STViewer::SwitchEye(int eye)
{
	m_images.BindTextures(m_shader, eye);
}

void STViewer::ReloadShaders()
{
	m_shader.Reload();
	m_images.BindTextures(m_shader, 0);
	m_images.BindTextures(m_shader, 1);
	m_shader.Bind();
}

std::vector<PanoInfo> STViewer::GetPanos()
{
	return m_panolist;
}

/* ---------------------- Camera Controls ---------------------- */

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


void STViewer::Screenshot()
{
	m_images.Screenshot(m_camera.Width, m_camera.Height);
}

void STViewer::FlipDebug()
{
	m_shader.FlipDebug();
}

void STViewer::ToggleGUI()
{
	m_displaygui = !m_displaygui;
}

void STViewer::ToggleLinear()
{
	m_linear = !m_linear;
	m_images.SetFilter(0, m_linear);
	if (m_stereo)
		m_images.SetFilter(1, m_linear);
}

/* ---------------------- Primary Update Loop ---------------------- */

void STViewer::Update(double globalTime, float deltaTime)
{
	if (m_remote != NULL && m_remote->ChangePano()) {
		if (m_images.InitPanoList(m_remote->GetPano())) {
			m_panolist = m_images.m_panoList;
			m_currentPano = 0;
			resetImages();
		}
	}

	if (m_usingVR)
	{
		updateVRDevice(&m_vr);
		VRControllerStates controllers = getVRControllerState(&m_vr);

		if (controllers.right.button1.pressed
			|| controllers.right.button2.pressed
			|| controllers.left.button1.pressed
			|| controllers.left.button2.pressed)
		{
			SelectPano(round(m_guiPanoSelection));
		}

		if (controllers.right.thumbstickTouch.down
			|| controllers.left.thumbstickTouch.down)
		{
			m_lastUIInteractionTime = globalTime;
		}

		if (controllers.right.thumbstickX != 0
			|| controllers.left.thumbstickX != 0)
		{
			m_lastUIInteractionTime = globalTime;
			float menuSpeed = 20;
			float analogExponent = 1.5f;
			float input = controllers.right.thumbstickX + controllers.left.thumbstickX;
			float moveAmount = powf(abs(input), analogExponent);
			if (input < 0) moveAmount *= -1;
			m_guiPanoSelection += moveAmount * deltaTime * menuSpeed;
			if (m_guiPanoSelection < 0) m_guiPanoSelection = 0;
			if (m_guiPanoSelection > m_panolist.size() - 1) m_guiPanoSelection = m_panolist.size() - 1;
		}
		else
		{
			// Lerp selection toward nearest integer
			float t = 10 * deltaTime;
			if (t > 1) t = 1;
			m_guiPanoSelection = (1 - t)*m_guiPanoSelection + t*round(m_guiPanoSelection);
		}
	}

	// 16.67ms per frame, takes us ~0.5ms to send an image to the GPU/Update quad depth (async GL calls)
	// Queue gets emptied fast enough that we never actually load 32 images sequentially, but we can do
	// Up to that many without skipping a frame.
	for (int i = 0; !m_LoadedTextures->IsEmpty() && i < 32; i++) {
		ImageData *image = m_LoadedTextures->Dequeue();
		//ImageData *image = m_

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
	m_LeftEye->RebindVAO();
	if (m_stereo)
		m_RightEye->RebindVAO();


	CB_Display();
	glutSwapBuffers();
}


/*---------------- Private Functions ----------------*/

void STViewer::loadAllQuadDepths()
{
	ImageHandler& images = m_images;
	for (int i = 0; i < downloadPool->size(); i++)
		downloadPool->submit([&images]() { images.LoadQuadImage(); });

	for (int i = 0; i < texturePool->size(); i++)
		texturePool->submit([&images]() { images.Decompress(); });
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
	if (m_LeftEye == NULL) {
		m_LeftEye = new CubePoints(m_maxDepth, 0);
		m_shader.SetFloatUniform("TileWidth", m_LeftEye->m_TILEWIDTH);
	}
	else {
		m_LeftEye->ResetDepth();
		m_LeftEye->RebindVAO();
		if (m_stereo) {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
	}
	
	CB_UpdateEyes(m_LeftEye, m_RightEye, m_stereo);

	// Wait for Level 0 to be loaded
	if (m_panolist.size() > 0)
		while (m_LoadedTextures->Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}

void STViewer::initGL()
{
	CB_InitReferences(m_stereo, &m_shader, &m_images, m_LeftEye, m_RightEye, &m_camera);
	CB_Init(this, m_fullscreen);
	CB_InitMenus(m_panolist);

	m_shader.CreateProgram("Shader.geom", "Shader.vert", "Shader.frag");
	m_shader.Bind();
}

void STViewer::initVR()
{
	m_usingVR = createVRDevice(&m_vr, m_camera.Width, m_camera.Height);
	if (m_usingVR) {
		m_stereo = true;
		updateVRDevice(&m_vr);
		CB_EnableVR(&m_vr);
	}
}

void STViewer::initTextures()
{
	m_images.InitTextureAtlas(m_stereo, m_LoadedTextures);

	// Trigger our logic pattern to init cubes and start loading images
	resetImages();

	// Left eye is default and always exists.
	// Texture bindings swap in CB_Display so no need to deal with Stereo mode here
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