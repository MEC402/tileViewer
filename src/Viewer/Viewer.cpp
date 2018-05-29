// Viewer.cpp : Defines the entry point for the console application.
//

#include <GL\glew.h>
#include <GL\freeglut.h>
#include <chrono>
#include <stdlib.h>
#include <time.h>
#include <vector>

#include "Camera.h"
#include "Controls.h"
#include "CubePoints.h"
#include "GLhandles.h"
#include "ShaderHelper.h"
#include "ImageHandler.h"
#include "ImageQueue.h"
#include "VR.h"
#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"

// Geometry data
CubePoints *cube;

// Vertex Buffer Data
GLuint program;
GLuint VBO;
GLuint VAO;
GLsizei pointCount;

VRDevice vrDevice = { 0 };
bool usingVR = false;


// TODO: Probably a better, less fragile way of tracking this data
// Keep track of quad depths locally
// Start depths at 1 since we automatically load in depth 0
int facedepths[6][2];
// Keep track of how many quads for a face we've loaded in (used to calculate if next face depth should load)
int facecount[6][2];

// Thread pools
Threads::ThreadPool tpool1_(std::thread::hardware_concurrency() / 4);
Threads::ThreadPool tpool2_(std::thread::hardware_concurrency() / 2);
std::deque<Threads::ThreadPool::TaskFuture<void>> v;

// Timer values for debugging
std::vector<std::chrono::high_resolution_clock::time_point> faceStart(6);
std::vector<std::chrono::high_resolution_clock::time_point> faceEnd(6);
std::chrono::high_resolution_clock::time_point t1;
std::chrono::high_resolution_clock::time_point t2;
long long duration;

// Depth for number of quads-per-face on our cube (powers of 2)
int imageResDepth = 3;

// State flag for poolhandler()
bool handling = false;

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
		glBindVertexArray(VAO);

		for (int i = 0; i < Camera::NumCameras; i++) {
			Camera::SetViewport(Camera::Cameras[i]);
		}

		//glDrawArrays(GL_POINTS, 0, pointCount);
		glFlush();
	}
}

// These two are no longer used but may be useful to have around in the future
/*
void LoadNextFaceDepth(int face)
{
	int faceCurDepth = cube->FaceCurrentDepth(face);
	if (faceCurDepth < imageResDepth) {
		ImageHandler::LoadFaceImage(face, faceCurDepth + 1);
	}
	else {
		fprintf(stderr, "Face %d is already at max depth\n", face);
	}
}

void LoadNextQuadDepth(int face, int row, int col)
{
	int quadCurDepth = cube->QuadCurrentDepth(face, row, col);
	if (quadCurDepth < imageResDepth) {
		ImageHandler::LoadQuadImage(face, row, col, quadCurDepth + 1);
	}
	else {
		fprintf(stderr, "Quad %d/%d is already at max depth\n", row, col);
	}
}
*/

void LoadQuadDepth(int face, int row, int col, int depth, int eye)
{
	if (depth < imageResDepth+1) {
		ImageHandler::LoadQuadImage(face, row, col, depth, eye);
	}
}

void LoadFace(int face, int eye)
{
	if (facedepths[face][eye] > 3) {
		faceEnd[face] = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(faceEnd[face] - faceStart[face]).count();
		fprintf(stderr, "Time to load face %d eye %d to max depth: %lld ms\n", face, eye, duration);
		return;
	}
	
	// Evidently it's faster for us to call new requests for each quad individually, since we have to decompress
	// the imagedata and calling ImageHandler::LoadFaceImage, which uses a thread to download concurrently,
	// will only have one thread for decompression/enqueueing
	// Dumping a lot of requests in effectively does every part of the process concurrently and we already spawned
	// the threads, so I don't see it being harmful

	int stepping = 8 / (int)pow(2, facedepths[face][eye]);
	for (int i = 0; i < 8; i += stepping) {
		for (int j = 7; j >= 0; j -= stepping) {
			v.emplace_back(tpool2_.submit([](int face, int row, int col, int depth, int eye)
			{
				LoadQuadDepth(face, row, col, depth, eye);
			}, face, i, j, facedepths[face][eye], eye));
		}
	}
	facedepths[face][eye]++;
}

void poolhandler()
{
	while (!v.empty()) {
		v.front().get();
		v.pop_front();
	}
	handling = false;
}

void generateCubePoints()
{
	cube = new CubePoints(imageResDepth);
	VBO = cube->m_PositionVBOID;
	VAO = cube->m_PositionVAOID;
	pointCount = cube->m_NumVertices;
	GLuint uTileWidth = glGetUniformLocation(program, "TileWidth");
	glUniform1f(uTileWidth, cube->m_TILEWIDTH);
}

// The intent of this is to use glutTimerFunc() to clear our Threadpool handles
// Not 100% sure it works as intended
void timerFunc(int value)
{
	if (!v.empty() && !handling) {
		handling = true;
		std::thread t(poolhandler);
		t.detach();
	}
	glutTimerFunc(2500, timerFunc, 0);
}

void idleFunc(void)
{
	if (usingVR) {
		updateVRDevice(&vrDevice);
	}

	// Load up to four images per frame
	for (int i = 0; i < 5; i++) {
		// Update one image/face group per frame at a time
		if (!ImageQueue::IsEmpty()) {
			ImageData *image = ImageQueue::Dequeue();

			// We're going to be deleting the image pointer reference in a second
			// Store these on the stack to make sure the quad updates
			int face = image->face;
			int row = image->row;
			int col = image->col;
			int depth = image->depth;
			int eye = image->eye;

			facecount[face][eye]++;

			if (facecount[face][eye] == (int)pow(4, facedepths[face][eye] - 1))
			{
				tpool1_.submit(LoadFace, face, eye);
				facecount[face][eye] = 0;
			}

			// TODO: Because we only have a single cube for drawing, one eye might load before the other
			// This causes the other eye to temporarily look at blank texture coordinates, resulting in black
			// squares
			// We need some kind of "only load if both are prepped" logic check
			tpool1_.submit(&CubePoints::QuadSetDepth, cube, face, row, col, depth);
			ImageHandler::LoadImageData(image);
		}
		else {
			break;
		}
	}

	// Check to see if any thread has updated cube quad depths
	// If so, rebind our VAO/VBO data
	if (cube->Ready) {
		cube->RebindVAO();
	}

	display();
	glutSwapBuffers();
}

int main(int argc, char * argv[])
{

	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	/* setup the size, position, and display mode for new windows */
	//glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);

	if (argc >= 3) {
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
	glutTimerFunc(1000, timerFunc, 1);
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

	if (argc >= 2) {
		ImageHandler::InitPanoListFromOnlineFile(argv[1]);
	}
	ImageHandler::InitTextureAtlas(program);


	generateCubePoints(); 
	Camera::Init(0);
	//Camera::UpdateMVP();
	//Camera::SetCameras();

	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	fprintf(stderr, "You should be seeing something on the screen now.\n");

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 2; j++) {
			facedepths[i][j] = 1;
		}
	}

	for (int i = 0; i < 6; i++)
		faceStart[i] = std::chrono::high_resolution_clock::now();
	

	glutMainLoop();
}