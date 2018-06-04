// Viewer.cpp : Defines the entry point for the console application.
//

// This file is a bona-fide mess.  One day it will be tidied.  Or not.

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

#define TIMERSTART t1 = std::chrono::high_resolution_clock::now();
#define TIMERSTOP std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
#define NOW std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - t1).count()
#define DURATION std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
#define LEFT_TIME fprintf(stderr, "Left Time: %lld us\n", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
#define RIGHT_TIME fprintf(stderr, "Right Time: %lld us\n", std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());

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

// Local face data
int facedepths[2][6];
int facecount[2][6];

// Thread pool /////////////////////////////
Threads::ThreadPool tpool_(std::thread::hardware_concurrency() - 1); // Use everything, whatever
Threads::ThreadPool tpool2_(1);
std::deque<Threads::ThreadPool::TaskFuture<void>> v; // So we can collect waiting promises from the threadpool
std::deque<Threads::ThreadPool::TaskFuture<void>> v2;

// Timer values for debugging /////////////
std::chrono::high_resolution_clock::time_point timerStart;
std::chrono::high_resolution_clock::time_point t1;
long long timerEnd[2][6];

// State flag for poolhandler() //////////
bool handling1 = false;
bool handling2 = false;

void drawString3D(const char *str, float pos[3], float color[4], void *font)
{
	glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
	glDisable(GL_LIGHTING);     // need to disable lighting for proper text color
	glDisable(GL_TEXTURE_2D);

	glColor4fv(color);          // set text color
	glRasterPos3fv(pos);        // place text position

								// loop all characters in the string
	while (*str)
	{
		glutBitmapCharacter(font, *str);
		++str;
	}

	glDisable(GL_TEXTURE_2D);
	glPopAttrib();
}

void showInfo()
{
	// backup current model-view matrix
	glPushMatrix();                     // save current modelview matrix
	glLoadIdentity();                   // reset modelview matrix

										// set to 2D orthogonal projection
	glMatrixMode(GL_PROJECTION);     // switch to projection matrix
	glPushMatrix();                  // save current projection matrix
	glLoadIdentity();                // reset projection matrix
	gluOrtho2D(0, Camera::Width, 0, Camera::Height); // set to orthogonal projection

	float color[4] = { 1, 1, 1, 1 };

	std::stringstream ss;
	ss << "Stereo: ";
	if (stereo)
		ss << "Enabled\n";
	else
		ss << "Disabled\n";

	ss << "Face completion times: ";
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 6; j++)
			if (timerEnd[i][j] != 0) {
				ss << "Face " << i << " " << j << " finished: ";
				ss << timerEnd[i][j] << "\n";
			}
	ss << std::ends;

	float pos[3] = {-1.0f, 1.0f, 0.0f };
	drawString3D(ss.str().c_str(), pos, color, GLUT_BITMAP_HELVETICA_18);
	ss.str(""); // clear buffer

	// restore projection matrix
	glPopMatrix();                   // restore to previous projection matrix

									 // restore modelview matrix
	glMatrixMode(GL_MODELVIEW);      // switch to modelview matrix
	glPopMatrix();                   // restore to previous modelview matrix
}

void timerFunc(int value)
{		
	if (!v.empty() && !handling1) {
		handling1 = true;
		std::thread t1(poolhandler1);
		t1.detach();
	}
	if (!v2.empty() && !handling2) {
		handling2 = true;
		std::thread t2(poolhandler2);
		t2.detach();
	}
	glutTimerFunc(5000, timerFunc, 0);
}

void poolhandler2()
{
	while (!v2.empty()) {
		try {
			v2.front().get();
		}
		catch (std::future_error& e){
			fprintf(stderr, "Caught a future error\n");
		}
		v2.pop_front();
	}
	handling2 = false;
}

void poolhandler1()
{
	while (!v.empty()) {
		try {
			v.front().get();
		}
		catch (std::future_error& e) {
			fprintf(stderr, "Caught a future error\n");
		}
		v.pop_front();
	}
	handling1 = false;
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

		//showInfo();

		glBindVertexArray(LeftEye->m_PositionVAOID);
		ImageHandler::RebindTextures(program, 0);
		for (int i = 0; i < Camera::NumCameras; i++) {
			Camera::SetViewport(Camera::LeftCameras[i]);
		}

		if (stereo) {
			glBindVertexArray(RightEye->m_PositionVAOID);
			ImageHandler::RebindTextures(program, 1);
			for (int i = 0; i < Camera::NumCameras; i++) {
				Camera::SetViewport(Camera::RightCameras[i]);
			}
		}

		//glDrawArrays(GL_POINTS, 0, pointCount);
		glFlush();
	}
}

void LoadFace(int face, int eye)
{
	if (facedepths[eye][face] > maxImageDepth)
		return;
	
	// Call each quad to load individually:
		// Pros:
			// -Faster to get each face in order fully loaded (2.2s for first face, 5s for last)
		// Cons:
			// -Almost guaranteed to load faces in sequential order (f,b,r,l,t,b) every time
			// -Much slower time between each face finishing
	int stepping = 8 / (int)pow(2, facedepths[eye][face]);
	for (int i = 0; i < 8; i += stepping) {
		for (int j = 7; j >= 0; j -= stepping) {
			v.emplace_back(tpool_.submit([](int face, int row, int col, int depth, int eye)
			{
				//LoadQuadDepth(face, row, col, depth, eye);
				ImageHandler::LoadQuadImage(face, row, col, depth, eye);
			}, face, i, j, facedepths[eye][face], eye));
		}
	}

	// Call each face to load in parallel:
		// Pros:
			// -Take advantage of libcurl's multi call function, less expensive to start new handles
			// -Faces actually get loaded in without guaranteed ordering, improving the "load in" effect
			// -Time difference between any given face being fully loaded is much, much smaller (100ms vs 200-800ms)
		// Cons:
			// -MUCH slower to get any given face fully loaded (4.8s for first, 6s for last)
			// -Has a tendency to load images in reverse order, which means we end up overwriting textures that quads in lower
				// depth levels still rely on, giving a very clear striped loading effect
	//v.emplace_back(tpool_.submit([](int face, int depth, int eye) {
	//	ImageHandler::LoadFaceImage(face, depth, eye);
	//}, face, facedepths[eye][face], eye));
	facedepths[eye][face]++;
}

void loadall()
{
	int eyes = 1;
	if (stereo)
		eyes = 2;

	// What the fuck
	for (int m = 0; m < 4; m++) {	
		int stepping = 8 / (int)pow(2, m);
		for (int i = 0; i < 8; i += stepping) {
			for (int j = 7; j >= 0; j -= stepping) {
				for (int k = 0; k < 6; k++) {
					for (int n = 0; n < eyes; n++) {
						v.emplace_back(tpool_.submit([](int face, int row, int col, int depth, int eye)
						{
							ImageHandler::LoadQuadImage(face, row, col, depth, eye);
						}, k, i, j, m, n));
					}
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

			// We're going to be deleting the image pointer reference in a second
					// Store these on the stack to make sure the quad updates
			int face = image->face;
			int row = image->row;
			int col = image->col;
			int depth = image->depth;
			int eye = image->eye;
			facecount[eye][face]++;
			if (depth == 3 && facecount[eye][face] == 85)
				fprintf(stderr, "Face %d finished in %lld ms!\n", face, NOW);
			//
			//if (facecount[eye][face] == (int)pow(4, facedepths[eye][face] - 1))
			//{
			//	v2.emplace_back(tpool2_.submit([](int face, int eye) {
			//		LoadFace(face, eye);
			//	}, face, eye));
			//	facecount[eye][face] = 0;
			//}

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

	// Check to see if any thread has updated cube quad depths
	// If so, rebind our VAO/VBO data
	if (LeftEye->Ready)
		LeftEye->RebindVAO();

	if (stereo && RightEye->Ready)
		RightEye->RebindVAO();


	display();
	glutSwapBuffers();
}

void prevPano()
{
	if (currentPanoIndex > 0) {
		currentPanoIndex--;
		ImageHandler::m_currentPano = currentPanoIndex;
		resetImages();
	}
	else {
		fprintf(stderr, "At first pano\n");
		return;
	}
}

void nextPano()
{
	if (currentPanoIndex < panolist.size() - 1) {
		currentPanoIndex++;
		ImageHandler::m_currentPano = currentPanoIndex;
		resetImages();
	}
	else {
		fprintf(stderr, "At last pano\n");
		return;
	}
}

void resetImages()
{
	tpool_.stopall();
	tpool2_.stopall();
	while (!ImageQueue::IsEmpty()) {
		ImageData *i = ImageQueue::Dequeue();
		free(i->data);
		free(i);
	}
	//ImageHandler::InitPanoListFromOnlineFile(panolist[currentPanoIndex]);
	//for (int i = 0; i < 6; i++) {
	//	ImageHandler::LoadQuadImage(i, 0, 0, 0, 0);
	//	if (stereo)
	//		ImageHandler::LoadQuadImage(i, 0, 0, 0, 1);
	//}
	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < 6; j++) {
			facedepths[i][j] = 1;
			facecount[i][j] = 0;
		}
	}
	resetCubes();
}

void resetCubes()
{
	if (LeftEye == NULL) {
		LeftEye = new CubePoints(maxImageDepth, 0);
		if (stereo)
			RightEye = new CubePoints(maxImageDepth, 1);
	}
	else {
		LeftEye->ResetDepth();
		LeftEye->RebindVAO();
		if (stereo) {
			RightEye->ResetDepth();
			RightEye->RebindVAO();
		}
	}
	std::thread t(loadall);
	t.detach();
	TIMERSTART
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

	// TODO: Unnecessary if we have a combined JSON file
	ImageHandler::InitPanoListFromOnlineFile(argv[argc-1]);
	panolist = ImageHandler::m_panoList;


	/* setup the size, position, and display mode for new windows */
	//glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);

	if (fullscreen) {
		glutGameModeString("width>=800 height>=600 bpp>=32 hertz>=60");
		// enter full screen
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
			glutEnterGameMode();
			Camera::Width = glutGameModeGet(GLUT_GAME_MODE_WIDTH);
			Camera::Height = glutGameModeGet(GLUT_GAME_MODE_HEIGHT);
			fprintf(stderr, "Width/Height set to %d %d\n", Camera::Width, Camera::Height);
		}
		else {
			printf("The select mode is not available\n");
			exit(1);
		}
	}
	else {
		/* create and set up a window */
		glutInitWindowSize(Camera::Width, Camera::Height);
		glutCreateWindow("Test Triangles");
	}

	

	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutSpecialFunc(Controls::ProcessGLUTKeys);
	glutKeyboardFunc(Controls::ProcessKeys);
	glutTimerFunc(3000, timerFunc, 1);
	//glutMouseFunc(mouseButton);
	glutMotionFunc(Controls::MouseMove);
	glutMouseWheelFunc(Controls::MouseWheel);
	//glutFullScreen();

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


	program = ShaderHelper::CreateProgram();
	ImageHandler::InitTextureAtlas(program, stereo);
	resetImages();

	//VBO = LeftEye->m_PositionVBOID;
	//VAO = LeftEye->m_PositionVAOID;
	pointCount = LeftEye->m_NumVertices;
	GLuint uTileWidth = glGetUniformLocation(program, "TileWidth");
	glUniform1f(uTileWidth, LeftEye->m_TILEWIDTH);
	//generateCubePoints(LeftEye);
	//generateCubePoints(RightEye);

	if (fivepanel)
		Camera::Init(5);
	else
		Camera::Init(1);

	if (stereo)
		Camera::SplitHorizontal();

	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	fprintf(stderr, "You should be seeing something on the screen now.\n");

	//loadall();
	
	timerStart = std::chrono::high_resolution_clock::now();
	
	glutMainLoop();
}