// STViewer.cpp : Runs the TileViewer using ST quad manipulations for texture loading
//

#include "STViewer.h"

#include "ImageQueue.h"

#include "Camera.h"
#include "Controls.h"

#ifdef DEBUG
#define TIMERSTART t1 = std::chrono::high_resolution_clock::now();
#define TIMERSTOP std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count()
#define DURATION std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
#endif


/*---------------- Public Functions ----------------*/

void STViewer::Init(const char* panoFileAddress, bool stereo, bool fivepanel, int viewportWidth, int viewportHeight)
{
	imagesNeedResetting = false;
	m_stereo = stereo;
	m_fivepanel = fivepanel;
	m_images.InitPanoList(panoFileAddress);

	// Init camera
	if (m_fivepanel)
		m_camera.Init(5, viewportWidth, viewportHeight);
	else
		m_camera.Init(1, viewportWidth, viewportHeight);
	if (m_stereo)
		m_camera.SplitHorizontal();

	texturePool = new Threads::ThreadPool(std::thread::hardware_concurrency());
	workerPool = new Threads::ThreadPool(2);

	m_panolist = m_images.m_panoList;
	m_currentPano = 0;

	m_maxDepth = 3; // Magic hardcoded number (powers of 2);

	m_LoadedTextures = new ImageQueue();

	initGL();

	// Attempt to create a VR device and enable it as appropriate
	m_usingVR = createVRDevice(&m_vr, m_camera.Width, m_camera.Height);
	if (m_usingVR) {
		m_stereo = true;
		updateVRDevice(&m_vr);
	}

	initTextures();
}

void STViewer::ToggleStereo()
{
	m_camera.SplitHorizontal();
	m_images.InitStereo();
	if (!m_stereo) {
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
	m_stereo = !m_stereo;
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

void STViewer::reloadShaders()
{
	m_shader.reload();
	m_images.bindTextures(m_shader, 0);
	m_images.bindTextures(m_shader, 1);
	m_shader.bind();
}

void STViewer::moveCamera(float pitchChange, float yawChange, float FOVChange)
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

std::vector<PanoInfo> STViewer::getPanoList()
{
	return m_panolist;
}

/*---------------- Private Functions ----------------*/

void STViewer::display()
{
	if (m_usingVR)
	{
		glDisable(GL_CULL_FACE);
		m_shader.setFloatUniform("TileWidth", m_LeftEye->m_TILEWIDTH);
		for (unsigned int eyeIndex = 0; eyeIndex < 2; ++eyeIndex) {
			m_shader.bind();
			bindEyeRenderSurface(&m_vr, eyeIndex);
			
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();

			m_images.bindTextures(m_shader, eyeIndex);

			glm::mat4x4 perspective = buildVRProjectionMatrix(&m_vr, eyeIndex);
			glm::mat4x4 view = buildVRViewMatrix(&m_vr, eyeIndex, 0, 0, 0);
			view = glm::translate(view, getVRHeadsetPosition(&m_vr)); // Negate headset translation
			m_shader.setMatrixUniform("MVP", perspective*view);

			if (eyeIndex == 0) glBindVertexArray(m_LeftEye->m_PositionVAOID);
			else glBindVertexArray(m_RightEye->m_PositionVAOID);
			glDrawArrays(GL_POINTS, 0, m_pointCount);

			commitEyeRenderSurface(&m_vr, eyeIndex);
		}
		finishVRFrame(&m_vr);
		blitHeadsetView(&m_vr, 0);
	}
	else
	{
		m_shader.bind();
		m_shader.setFloatUniform("TileWidth", m_LeftEye->m_TILEWIDTH);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	
		m_images.bindTextures(m_shader, 0);
		glBindVertexArray(m_LeftEye->m_PositionVAOID);
		for (int i = 0; i < m_camera.NumCameras; i++) {
			m_camera.SetViewport(m_camera.LeftCameras[i]);
			m_shader.setMatrixUniform("MVP", m_camera.MVP);
			glDrawArrays(GL_POINTS, 0, m_pointCount);
		}

		if (m_stereo) {
			glBindVertexArray(m_RightEye->m_PositionVAOID);
			m_images.bindTextures(m_shader, 1);
			for (int i = 0; i < m_camera.NumCameras; i++) {
				m_camera.SetViewport(m_camera.RightCameras[i]);
				m_shader.setMatrixUniform("MVP", m_camera.MVP);
				glDrawArrays(GL_POINTS, 0, m_pointCount);
			}
		}

		glFlush();
	}
}

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
	for (int i = 0; i < texturePool->size() / 2; i++) {
		texturePool->submit([&images]() { images.LoadQuadImage(); });
		texturePool->submit([&images]() { images.Decompress(); });
	}
}

void STViewer::update(void)
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
	texturePool->stopall();
	workerPool->stopall();

	// Clear out everything that's still sitting in our queue and enable discarding of new items
	m_LoadedTextures->Clear();
	m_images.ClearQueues();

	// Wait for any threads that are mid-work to finish
	while (!texturePool->allstopped());

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
	imagesNeedResetting = false;
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
	
	// Wait for Level 0 to be loaded
	while (m_LoadedTextures->Size() < 6);

#ifdef DEBUG
	TIMERSTART
#endif
}

void STViewer::resize(int w, int h)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	m_camera.Width = w;
	m_camera.Height = h;
	fprintf(stderr, "%d %d\n", m_camera.Width, m_camera.Height);
	m_camera.UpdateMVP();
	m_camera.UpdateCameras();
	if (m_usingVR) {
		resizeMirrorTexture(&m_vr, w, h);
	}
}

void STViewer::initGL()
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	m_shader.createProgram("Shader.geom", "Shader.vert", "Shader.frag");
	m_shader.bind();
}

void STViewer::initTextures()
{
	m_images.InitTextureAtlas(m_stereo, m_LoadedTextures);

	// Trigger our logic pattern to init cubes and start loading images
	resetImages();

	m_pointCount = m_LeftEye->m_NumVertices;

	fprintf(stderr, "ST Viewer Initialized\n");
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

void STViewer::RebindVAO()
{
	m_LeftEye->RebindVAO();
}

#endif