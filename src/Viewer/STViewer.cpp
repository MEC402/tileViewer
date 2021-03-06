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

STViewer::STViewer(const char* panoURI, bool stereo, bool fivepanel, bool fullscreen, bool borderless,
	int viewWidth, int viewHeight, RemoteServent *remote, KinectControl *kinect) :
	m_stereo(stereo),
	m_fivepanel(fivepanel),
	m_fullscreen(fullscreen),
	m_borderless(borderless),
	m_remote(remote),
	m_kinect(kinect)
{
	PRINT_DEBUG_MESSAGE("Initializing Panolist");
	m_images.InitPanoList(panoURI);

	PRINT_DEBUG_MESSAGE("Intiailizing Camera");
	if (m_fivepanel)
		m_camera.Init(5, viewWidth, viewHeight);
	else
		m_camera.Init(1, viewWidth, viewHeight);

	if (m_stereo) {
		PRINT_DEBUG_MESSAGE("Initializing Stereo");
		m_camera.SplitHorizontal();
	}

	PRINT_DEBUG_MESSAGE("Starting threadpools");
	downloadPool = new Threads::ThreadPool(std::thread::hardware_concurrency());
	texturePool = new Threads::ThreadPool(std::thread::hardware_concurrency()-1);
	workerPool = new Threads::ThreadPool(2);

	m_panolist = m_images.m_panoList;

	m_LoadedTextures = new SafeQueue<ImageData*>();

	PRINT_DEBUG_MESSAGE("Initializing GL callbacks");
	initGL();
	initVR();
	PRINT_DEBUG_MESSAGE("Initializing Textures");
	initTextures();

#ifdef OBJLOAD
	m_objloader.LoadObj("C:\\Users\\W8\\Desktop\\teapot.obj", m_objdata);
#endif
	PRINT_DEBUG_MESSAGE("Initializing Annotations and GUI")
	m_annotations.Create("en");
	workerPool->submit([&]() { m_gui.Create(m_panolist); });

	Controls::SetViewer(this);

	if (m_remote != NULL
		&& !m_remote->m_Serving
		&& m_remote->m_DistributedView)
	{
		m_remote->SetCameraPtr(&m_camera);
	}

	PRINT_DEBUG_MESSAGE("Starting main loop");
	glutMainLoop();
}

void STViewer::ToggleStereo()
{
	m_stereo = !m_stereo;
	m_camera.SplitHorizontal();
	m_images.InitStereo();
	if (m_stereo) {
		if (m_RightEye == NULL) {
			m_RightEye = new CubePoints(m_maxDepth, RIGHT_EYE);
		}
		else {
			m_RightEye->ResetDepth();
			m_RightEye->RebindVAO();
		}
		m_camera.UpdateCameras();
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
		if (m_guistate == PANO) {
			m_selectedPano = (float)pano;
		}
		else {
			m_currentPano = pano;
			m_guiPanoSelection = (float)pano;
			m_selectedPano = (float)pano;
			m_images.m_currentPano = m_currentPano;
			if (m_remote != NULL && m_remote->m_Serving)
				m_remote->ChangePano(m_currentPano);

			resetImages();
		}
	}
	else {
		fprintf(stderr, "Invalid Pano Selection\n");
	}
}

void STViewer::ToggleComparison()
{
	m_comparisonMode = !m_comparisonMode;
	if (!m_comparisonMode) {
		ToggleEye(LEFT_EYE); // Set back to Left Eye textures
		return;
	}
	m_images.CopyImageData();
	ToggleGUI(PANO);
}

void STViewer::ToggleEye(int eye)
{
	m_images.BindTextures(m_shader, eye);
}

void STViewer::ReloadShaders()
{
	m_shader.Reload();
	m_images.BindTextures(m_shader, LEFT_EYE);
	if (m_stereo)
		m_images.BindTextures(m_shader, RIGHT_EYE);
	m_shader.Bind();
}

std::vector<PanoInfo> STViewer::GetPanos()
{
	return m_panolist;
}

PanoInfo STViewer::GetCurrentPano()
{
	return m_panolist[m_currentPano];
}

/* ---------------------- Camera Controls ---------------------- */


void STViewer::MoveCamera(float pitchChange, float yawChange, float FOVChange)
{
	m_camera.MoveCamera(pitchChange, yawChange, FOVChange);
	if (m_remote != NULL && m_remote->m_Serving) {
		if (m_remote->m_DistributedView) {
			float xFOV, yFOV;
			_camera->GetFOV(xFOV, yFOV);
			m_remote->UpdateClients(m_camera.GetYaw(), m_camera.GetPitch(), yFOV);
			//m_remote->UpdateClients(m_camera.GetYaw(), m_camera.GetPitch(), xFOV, yFOV);
		}
		else {
			m_remote->UpdateClients(m_camera.GetYaw(), m_camera.GetPitch());
		}
	}
}

void STViewer::ResetCamera()
{
	m_camera.ResetCamera();
}

void STViewer::ToggleExactPixels()
{
	m_exactpixelmode = !m_exactpixelmode;
	if (m_exactpixelmode)
		m_camera.SetPixelPerfect();
	else
		ResetCamera();
}

void STViewer::Screenshot()
{
	m_images.Screenshot(m_camera.ScreenWidth, m_camera.ScreenHeight);
}

void STViewer::ToggleDebug()
{
	m_shader.ToggleDebug();
}

void STViewer::ToggleGUI(GUISTATE state)
{
	if (m_guistate == OFF) {
		m_guistate = state;
		m_gui.StartTimer();
	}
	else {
		m_guistate = OFF;
		m_gui.ResetTimer();
	}
}

void STViewer::ToggleLinear()
{
	m_linear = !m_linear;
	m_images.SetFilter(LEFT_EYE, m_linear);
	if (m_stereo)
		m_images.SetFilter(RIGHT_EYE, m_linear);
}

void STViewer::ToggleObj()
{
#ifdef OBJLOAD
	m_displayObj = !m_displayObj;
#endif
	return;
}

bool STViewer::SetDisplayStates()
{

	if (m_guistate == PANO)
		displayGUI();
	if (m_displayAnnotation)
		displayAnnotations();
#ifdef OBJLOAD
	if (m_displayObj)
		displayObjs();

	return (m_guistate == PANO || m_displayAnnotation || m_displayObj);
#else
	return (m_guistate == PANO || m_displayAnnotation);
#endif
}

/* ---------------------- Primary Update Loop ---------------------- */

// TODO: This is getting quite lengthy, possibly break up and refactor for readability/maintainability?
void STViewer::Update(double globalTime, float deltaTime)
{
	if (m_remote != NULL) {
		if (m_remote->ChangePano()) {
			SelectPano(m_remote->GetPanoIndex());
		} 
		if (!m_remote->m_Serving && m_remote->m_Update) {
			float yaw, pitch;
			m_remote->GetCameraUpdate(yaw, pitch);
			m_camera.SetCamera(pitch, yaw);
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
			SelectPano((int)round(m_guiPanoSelection));
		}

		if (controllers.right.thumbstickTouch.down
			|| controllers.left.thumbstickTouch.down)
		{
			m_lastUIInteractionTime = globalTime;
		}

		if (controllers.right.thumbstickX != 0
			|| controllers.left.thumbstickX != 0)
		{
			if (controllers.right.indexFingerTrigger > 0.05f)
			{
				// Adjust panorama alignment
				float analogExponent = 2.0f;
				float moveSpeed = deltaTime * 1.0f;
				float hChange = moveSpeed * powf(controllers.right.thumbstickX, analogExponent);
				if (controllers.right.thumbstickX < 0) hChange *= -1;
				_horizontalEyeRotation += hChange;
				float vChange = moveSpeed * powf(controllers.right.thumbstickY, analogExponent);
				if (controllers.right.thumbstickY < 0) vChange *= -1;
				_verticalEyeRotation += vChange;

			}
			else
			{
				// Change GUI menu selection
				m_lastUIInteractionTime = globalTime;
				float menuSpeed = 20;
				float analogExponent = 1.5f;
				float input = controllers.right.thumbstickX + controllers.left.thumbstickX;
				float moveAmount = powf(abs(input), analogExponent);
				if (input < 0) moveAmount *= -1;
				m_guiPanoSelection += moveAmount * deltaTime * menuSpeed;
				if (m_guiPanoSelection < 0) m_guiPanoSelection = 0;
				if (m_guiPanoSelection > m_panolist.size() - 1) m_guiPanoSelection = (float)m_panolist.size() - 1;
			}
		}
		else
		{
			// Lerp selection toward nearest integer
			float t = 10 * deltaTime;
			if (t > 1) t = 1;
			m_guiPanoSelection = (1 - t)*m_guiPanoSelection + t*round(m_guiPanoSelection);
		}
	}
	else {
		// Animation for non-VR GUI
		if ((m_guistate == PANO) && abs(m_guiPanoSelection - m_selectedPano) > 0.01) {
			float menuSpeed = 20;
			float direction = m_selectedPano - m_guiPanoSelection;

			if (direction < 0)
				m_guiPanoSelection += -0.2f * deltaTime * menuSpeed * -direction;
			else
				m_guiPanoSelection += 0.2f * deltaTime * menuSpeed * direction;

			if (m_guiPanoSelection < 0)
				m_guiPanoSelection = 0;

			if (m_guiPanoSelection > m_panolist.size() - 1) 
				m_guiPanoSelection = (float)m_panolist.size() - 1;
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
			m_average.push_back((float)now);
		}
#endif

		if (eye == LEFT_EYE) {
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

	if (m_kinect != NULL) {
		m_kinect->GetGesture(m_guistate == PANO);
	}

	// Check to see if any thread has updated cube quad depths, if so rebind VAO/VBO data
	m_LeftEye->RebindVAO();
	if (m_stereo)
		m_RightEye->RebindVAO();

	CB_Display();
	glutSwapBuffers();
}


/*---------------- Private Functions ----------------*/

void STViewer::resetImages()
{
	workerPool->stopall();

	// Clear out everything that's still sitting in our queue and enable discarding of new items
	m_LoadedTextures->Clear();
	m_images.ClearQueues();

	// Clears previous annotations loads annotations for the new pano, if any
	m_annotations.Load(m_panolist[m_currentPano].annotations, "en");

	// Sanity check
	m_images.ClearQueues();

	/* Turn off queue discarding */
	m_LoadedTextures->ToggleDiscard();

	m_images.InitURLs(m_currentPano, m_stereo);
	
#ifdef DEBUG
	// These aren't really important anymore but might be useful for debugging
	// We can spare the 0.01ms it takes to reset them
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			m_facecount[i][j] = 0;
		}
	}
#endif

	resetCubes();
}

void STViewer::resetCubes()
{
	// Out of order logic so we can toggle Stereo on during runtime and properly generate the RightEye
	if (m_stereo && m_RightEye == NULL)
		m_RightEye = new CubePoints(m_maxDepth, RIGHT_EYE);

	// LeftEye is default
	if (m_LeftEye == NULL) {
		m_LeftEye = new CubePoints(m_maxDepth, LEFT_EYE);
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

	// Wait for Level 0 to be loaded so we don't get huge, zoomed in textures
	if (m_panolist.size() > 0)
		while (m_LoadedTextures->Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}


/*---------------- OpenGL Init Functions ----------------*/

void STViewer::initGL()
{
	CB_InitReferences(m_stereo, &m_shader, &m_objectShader,	&m_images, 
		m_LeftEye, m_RightEye, &m_camera);
	CB_InitMenus(m_panolist);
	CB_Init(this, m_fullscreen, m_borderless);

	m_shader.CreateProgram("Shader.geom", "Shader.vert", "Shader.frag");
	m_objectShader.CreateProgram(0, "gui.vert", "gui.frag");
#ifdef OBJLOAD
	m_objshader.CreateProgram(NULL, "obj.vert", "obj.frag");
#endif
	m_shader.Bind();
}

void STViewer::initVR()
{
	m_usingVR = createVRDevice(&m_vr, m_camera.ScreenWidth, m_camera.ScreenHeight);
	if (m_usingVR) {
		PRINT_DEBUG_MESSAGE("Initializing VR");
		m_stereo = true;
		updateVRDevice(&m_vr);
		CB_EnableVR(&m_vr);
	}
}

void STViewer::initTextures()
{
	m_images.InitTextureAtlas(m_stereo, m_LoadedTextures);
	
	// Initialize our threadpools to wait on blocking cond_var mutex calls
	// This way we don't have to requeue the calls over and over or worry about
	// threadpools doubling up and causing system slowdowns
	ImageHandler& images = m_images;
	for (int i = 0; i < downloadPool->size(); i++)
		downloadPool->submit([&images]() { images.LoadQuadImage(); });
	for (int i = 0; i < texturePool->size(); i++)
		texturePool->submit([&images]() { images.Decompress(); });

	// Trigger our logic pattern to init cubes and start loading images
	resetImages();
	

	// Left eye is default and always exists.
	// Texture bindings swap in CB_Display so no need to deal with Stereo mode here
	m_pointCount = m_LeftEye->m_NumVertices;
	m_images.BindTextures(m_shader, 0);
	
	fprintf(stderr, "ST Viewer Initialized\n");
}


/*---------------- Various Display/Draw Functions ----------------*/

void STViewer::displayAnnotations()
{
	glm::mat4 correctedView = m_camera.View * glm::rotate(-glm::pi<float>()/2, glm::vec3(0, 1, 0));
	m_annotations.Display(m_camera.Projection, correctedView, &m_objectShader, 1, false);
	if (m_stereo)
		m_annotations.Display(m_camera.Projection, correctedView, &m_objectShader, 1, false);
}

void STViewer::displayGUI()
{
	if (!m_gui.FinishedLoading)
		return;

	m_gui.LoadThumbnailTextures();
	// Draw a new viewport to cover the whole scene
	glViewport(0, 0, m_camera.ScreenWidth, m_camera.ScreenHeight);
	// Reset projection to "only one screen"
	glm::mat4 proj = glm::perspective(glm::radians(45.0f),
		float(m_camera.ScreenWidth) / float(m_camera.ScreenHeight), 0.1f, 10000.0f);
	m_gui.Display(glm::quat(glm::inverse(m_camera.View)),
		proj * m_camera.View, &m_objectShader, m_guiPanoSelection);
}

void STViewer::displayObjs()
{
#ifdef OBJLOAD
	m_objloader.DrawObj(m_objdata, &m_objshader, m_camera.Projection*m_camera.View);
#endif
	return;
}


void STViewer::Cleanup()
{
	downloadPool->stopall();
	texturePool->stopall();
	workerPool->stopall();
	m_LoadedTextures->Clear();
	m_images.ClearQueues();
}

#ifdef DEBUG

void STViewer::RebindVAO()
{
	m_LeftEye->RebindVAO();
}

#endif