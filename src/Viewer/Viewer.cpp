// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include "VR.h"
#include "ThreadPool.hpp"
#include "ThreadSafeQueue.hpp"

// Defaults for window height/width
int width = 1280;
int height = 800;

// This will identify our vertex buffer
GLuint VBO;
GLuint VAO;
GLsizei pointCount;
GLuint program;

// Geometry data
CubePoints *cube;

// Matricies
glm::mat4 Projection;
glm::mat4 View;
glm::mat4 Model;

VRDevice vrDevice = { 0 };
bool usingVR = false;

// Camera rotation stuff
bool firstMouse = true;
float yaw = -270.0f;
float pitch = 0.0f;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
float zoom = 0.0f;
float fov = 34.8093072;


// 5x (1080x1920) -> Aspect ratio 2.8125
// 1920x108 -> Aspect ratio 1.777...

// Keep track of quad depths locally
// Start depths at 1 since we automatically load in depth 0
int facedepths[6] = { 1,1,1,1,1,1 };
// Keep track of how many quads for a face we've loaded in (used to calculate if next face depth should load)
int facecount[6] = { 0 };

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

// Debug flag
bool DEBUG = false;

// Debugging row/col values so we can update specific quads
int DEBUG_row = 0;
int DEBUG_col = 0;
float DEBUG_camerastep = 1.0f;
float DEBUG_fov = 34.8093072;

// Values to mess with multiple viewports
float degreeshift[5] = { 0.0f };
int numcameras = 5;
float widthoffset = 0.2f;

struct Camera {
	int leftcorner;
	int width;
	int height;
	float rotation;
};

Camera *cameras[5];

void updatematrix();


void setviewport(Camera *camera)
{
	glm::mat4 newModel = glm::rotate(Model, camera->rotation, glm::vec3(0, 1, 0));
	glm::mat4 mvp = Projection * View * newModel;
	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);

	glViewport(camera->leftcorner, 0, camera->width, camera->height);
	glDrawArrays(GL_POINTS, 0, pointCount);
}

void SetCameras(Camera **cams, double fovy, double aRatio, bool multiscreen)
{
	//if (cams != NULL) {
	//	delete cams;
	//}

	//setupViewScreens(aviewer, 34.8093072, 1080.0/1920.0,multiscreen);
	//void setupViewScreens(osgViewer::Viewer *aviewer, double fovy, double aRatio, bool multiscreen)
	//   double fovx = atan(tan(osg::DegreesToRadians(fovy*0.5)) * aRatio) * 2.0;
	//unsigned int numScreens = 1;
	//double rotate_x = 0.0;
	//std::cerr << "Numscreens = " << numScreens << std::endl;
	//if (multiscreen)
	//{
	//	numScreens = 5;
	//	rotate_x = -double(numScreens - 1) * 0.5 * fovx;
	//}
	//for (unsigned int i = 0; i<numScreens; ++i, rotate_x += fovx)
	//{
	//	std::cerr << "Camera #" << i << " " << (width / numScreens)*i << std::endl;
	//	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
	//	camera->setGraphicsContext(gc.get());
	//	int lcorn = (width / numScreens);
	//	std::cerr << "Lcorn " << lcorn << " " << lcorn * i << std::endl;
	//	camera->setViewport(new osg::Viewport(lcorn*i, 0, lcorn, height));
	//	aviewer->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(rotate_x, 0.0, 1.0, 0.0));
	//
	//}
	
	double fovx = glm::atan(glm::tan(glm::radians(fovy*0.5)) * aRatio) * 2.0;
	unsigned int numScreens = 1;
	float rotate_x = 0.0;
	if (multiscreen) {
		numScreens = 5;
		rotate_x = -double(numScreens - 1) * 0.5 * fovx;
		for (int i = 0; i < numScreens; i++) {
			cams[i] = new Camera{ 0 };
		}
	}
	else {
		cams = new Camera*[1];
		cams[0] = new Camera{ 0 };
	}

	for (unsigned int i = 0; i < numScreens; ++i, rotate_x += fovx) {
		fprintf(stderr, "Camera %d %f\n", i, (width / numScreens));
		//cams[i] = Camera{ 0 };
		cams[i]->leftcorner = ((float)width / numScreens) * i;
		cams[i]->width = ((float)width / numScreens);
		cams[i]->height = height;
		cams[i]->rotation = rotate_x;
	}
}

void shiftmatrix(float degrees)
{

	float fovx = glm::atan(glm::tan(glm::radians(fov * 0.5)) * (height / (width * widthoffset))) * 2.0;
	float rotate_x = -float(4) * 0.5f * fovx;
	rotate_x += (degrees * fovx);

	glm::mat4 newModel = glm::rotate(Model, glm::degrees(rotate_x), glm::vec3(0, 1, 0));
	//glm::mat4 newModel = glm::rotate(Model, rotate_x + degrees, glm::vec3(0, 1, 0));
	glm::mat4 mvp = Projection * View * newModel;

	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
	//glm::mat4 newModel = glm::rotate(Model, fovy + degrees, glm::vec3(0, 1, 0));
	////glm::mat4 newProjection = glm::perspective(fovy, (float)(height / (width * widthoffset)), 0.1f, 100.0f);
	//glm::mat4 mvp = newProjection * View * newModel;
	//GLuint MatrixID = glGetUniformLocation(program, "MVP");
	//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
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
		glBindVertexArray(VAO);

		updatematrix();

		for (int i = 0; i < 5; i++) {
			setviewport(cameras[i]);
		}

		//glDrawArrays(GL_POINTS, 0, pointCount);
		glFlush();
	}
}

void updatematrix()
{
	//Projection = glm::perspective(glm::radians(fov), (float)((width * widthoffset) / height), 0.1f, 100.0f);
	Projection = glm::perspective(glm::radians(fov), (float)(float(height) / float(width)), 0.1f, 10000.0f);
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	glm::vec3 cameraFront = glm::normalize(front);

	View = glm::lookAt(
		glm::vec3(0, 0, 0),
		cameraFront, //glm::vec3(x, y, z),
		glm::vec3(0, 1, 0)
	);
	Model = glm::mat4(1.0f);
	//Model = glm::rotate(Model, glm::radians(30.0f), glm::vec3(x, y, z));
	glm::mat4 mvp = Projection * View * Model;

	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
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

void perspective(int minx, int maxx, int miny, int maxy, int minz, int maxz)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(minx, maxx, miny, maxy, minz, maxz);
}

void perspective(int angle, float aspectRatio, float min, float max)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(angle, aspectRatio, min, max);
}

void perspective()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, (width * widthoffset) / height, 0.1f, 100.0f);
}

void flipDebug()
{
	GLuint uDebug = glGetUniformLocation(program, "Debug");
	glUniform1f(uDebug, DEBUG);
}

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

void LoadQuadDepth(int face, int row, int col, int depth)
{
	if (depth < imageResDepth+1) {
		ImageHandler::LoadQuadImage(face, row, col, depth);
	}
}

void mouseMove(int posx, int posy)
{
	//fprintf(stderr, "Mouse movement %d , %d", posx, posy);
	if (firstMouse)
	{
		lastX = (float)posx;
		lastY = (float)posy;
		firstMouse = false;
	}

	float xoffset = posx - lastX;
	float yoffset = lastY - posy; // reversed since y-coordinates go from bottom to top
	lastX = (float)posx;
	lastY = (float)posy;

	float sensitivity = 0.01f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;
	updatematrix();
	lastX = width / 2.0f;
	lastY = height / 2.0f;
}

void mouseWheel(int button, int dir, int x, int y)
{
	// Zoom in/out by manipulating fov and updating MVP matrix
	if (dir > 0)
	{
		fov -= 2.0f;
		if (fov < 0) {
			fov = 1.0f;
		}
	}
	else
	{
		fov += 2.0f;
	}
	updatematrix();
	SetCameras(cameras, fov, ((double)height / (double)width), true);
}


// Aside from up/down/right/left these functions are all for debugging
void processGlutKeys(int key, int x1, int y1)
{
	switch (key) {
	case GLUT_KEY_UP:
		pitch += 1.0f;
		break;

	case GLUT_KEY_DOWN:
		pitch -= 1.0f;
		break;

	case GLUT_KEY_RIGHT:
		yaw += 1.0f;
		break;

	case GLUT_KEY_LEFT:
		yaw -= 1.0f;
		break;

	case GLUT_KEY_PAGE_UP:
		fov += 0.1f;
		//if (DEBUG_row >= 7) {
		//	DEBUG_row = 0;
		//}
		//else {
		//	DEBUG_row++;
		//}
		//fprintf(stderr, "Select row/col to increase depth: %d / %d\n", DEBUG_row, DEBUG_col);
		break;

	case GLUT_KEY_PAGE_DOWN:
		fov -= 0.1f;
		//if (DEBUG_col >= 7) {
		//	DEBUG_col = 0;
		//}
		//else {
		//	DEBUG_col++;
		//}
		//fprintf(stderr, "Select row/col to increase depth: %d / %d\n", DEBUG_row, DEBUG_col);
		break;

	case GLUT_KEY_F1:
		program = ShaderHelper::ReloadShader(GL_VERTEX_SHADER);
		ImageHandler::RebindTextures(program);
		break;
	case GLUT_KEY_F2:
		program = ShaderHelper::ReloadShader(GL_GEOMETRY_SHADER);
		ImageHandler::RebindTextures(program);
		break;
	case GLUT_KEY_F3:
		program = ShaderHelper::ReloadShader(GL_FRAGMENT_SHADER);
		ImageHandler::RebindTextures(program);
		break;

	case GLUT_KEY_F4:
		SetCameras(cameras, fov, ((double)height / (double)width), true);
		break;

	case GLUT_KEY_F5:
		fprintf(stderr, "FOV is at %f\n", fov);
		for (int i = 0; i < 5; i++) {
			fprintf(stderr, "Camera %d is shifted %f degrees\n", i, degreeshift[i]);
		}
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
		DEBUG = !DEBUG;
		flipDebug();
		break;

	case GLUT_KEY_F9:
		updatematrix();
		break;

	case GLUT_KEY_F10:
		//zoffset -= step;
		break;

	case GLUT_KEY_F12:
		//Threads::DefaultThreadPool::submitJob(LoadFaceByQuads, 0);
		break;
	}
}

bool handling = false;
void poolhandler()
{
	while (!v.empty()) {
		v.front().get();
		v.pop_front();
	}
	handling = false;
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

void processKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		degreeshift[0] += DEBUG_camerastep;
		break;
	case '2':
		degreeshift[0] -= DEBUG_camerastep;
		break;
	case '3':
		degreeshift[1] += DEBUG_camerastep;
		break;
	case '4':
		degreeshift[1] -= DEBUG_camerastep;
		break;
	case '5':
		degreeshift[2] += DEBUG_camerastep;
		break;
	case '6':
		degreeshift[2] -= DEBUG_camerastep;
		break;
	case '7':
		degreeshift[3] += DEBUG_camerastep;
		break;
	case '8':
		degreeshift[3] -= DEBUG_camerastep;
		break;
	case '9':
		degreeshift[4] += DEBUG_camerastep;
		break;
	case '0':
		degreeshift[4] -= DEBUG_camerastep;
		break;

	case 'r':
	case 'R':
		fov = DEBUG_fov;
		pitch = 0.0f;
		updatematrix();
		SetCameras(cameras, fov, ((double)height / (double)width), true);
		break;

	case 27:
		//glutLeaveFullScreen();
		glutLeaveGameMode();
		break;
	}
}

void LoadFace(int face)
{
	if (facedepths[face] > 3) {
		faceEnd[face] = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::milliseconds>(faceEnd[face] - faceStart[face]).count();
		fprintf(stderr, "Time to load face %d to max depth: %lld ms\n", face, duration);
		return;
	}
	
	// Evidently it's faster for us to call new requests for each quad individually, since we have to decompress
	// the imagedata and calling ImageHandler::LoadFaceImage, which uses a thread to download concurrently,
	// will only have one thread for decompression/enqueueing
	// Dumping a lot of requests in effectively does every part of the process concurrently and we already spawned
	// the threads, so I don't see it being harmful

	//v.emplace_back(tpool2_.submit([](int face, int depth) 
	//{
	//	ImageHandler::LoadFaceImage(face, depth);
	//}, face, facedepths[face]));
	int stepping = 8 / (int)pow(2, facedepths[face]);
	for (int i = 0; i < 8; i += stepping) {
		for (int j = 7; j >= 0; j -= stepping) {
			v.emplace_back(tpool2_.submit([](int face, int row, int col, int depth)
			{
				LoadQuadDepth(face, row, col, depth);
			}, face, i, j, facedepths[face]));
		}
	}
	facedepths[face]++;
}

void idleFunc(void)
{
	if (usingVR) {
		updateVRDevice(&vrDevice);
	}

	// Load up to three images per frame
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
			facecount[face]++;
			if (facecount[face] == (int)pow(4, facedepths[face] - 1))
			{
				//std::thread t(LoadFace, face);
				//t.detach();
				tpool1_.submit(LoadFace, face);
				facecount[face] = 0;
			}
			//tpool1_.submit(&CubePoints::QuadNextDepth, cube, face, row, col);
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

	glutGameModeString("width>=800 height>=600 bpp>=32 hertz>=60");
	// enter full screen
	if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE)) {
		glutEnterGameMode();
		width = glutGameModeGet(GLUT_GAME_MODE_WIDTH);
		height = glutGameModeGet(GLUT_GAME_MODE_HEIGHT);
		fprintf(stderr, "Width/Height set to %d %d\n", width, height);
	}
	else {
		printf("The select mode is not available\n");
		exit(1);
	}


	/* create and set up a window */
	//glutInitWindowSize(width, height);
	//glutCreateWindow("Test Triangles");

	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutSpecialFunc(processGlutKeys);
	glutKeyboardFunc(processKeys);
	glutTimerFunc(1000, timerFunc, 1);
	//glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutMouseWheelFunc(mouseWheel);
	//glutFullScreen();

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Attempt to create a VR device and enable it as appropriate
	usingVR = createVRDevice(&vrDevice, width, height);
	if (usingVR) {
		updateVRDevice(&vrDevice);
	}


	program = ShaderHelper::CreateProgram();

	if (argc >= 2) {
		ImageHandler::InitPanoListFromOnlineFile(argv[1]);
	}
	ImageHandler::InitTextureAtlas(program);


	flipDebug();
	generateCubePoints();
	updatematrix();

	/* define the projection transformation */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(fov, (width * widthoffset) / height, 0.1f, 100.0f);
	glViewport(0, 0, width, height);

	fprintf(stderr, "You should be seeing something on the screen now.\n");

	for (int i = 0; i < 6; i++)
		faceStart[i] = std::chrono::high_resolution_clock::now();

	//setupViewScreens(aviewer, 34.8093072, 1080.0/1920.0,multiscreen);
	SetCameras(cameras, fov, double((double)height / (double)width), true);

	glutMainLoop();
}