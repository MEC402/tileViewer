// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>

int width = 1024;
int height = 800;

// This will identify our vertex buffer
GLuint VBO;
GLuint VAO;
GLsizei pointCount;
GLuint program;

// Geometry data
CubePoints *cube;

// Camera rotation stuff
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = width / 2.0f;
float lastY = height / 2.0f;
float zoom = 0.0f;
float fov = 45.0f;

float scale = 1.0f;

// Depth for number of quads-per-face on our cube (powers of 2)
int imageResDepth = 3;

// Debug flag
bool DEBUG = false;

void display()
{
	// The worlds most basic draw routine!
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glBindVertexArray(VAO);
	
	glDrawArrays(GL_POINTS, 0, pointCount);
	glFlush();
}

void updatematrix()
{
	glm::mat4 Project = glm::perspective(glm::radians(45.0f), (float)(width / height), 0.1f, 100.0f);


	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	glm::vec3 cameraFront = glm::normalize(front);

	glm::mat4 View = glm::lookAt(
		glm::vec3(0, 0, 0),
		cameraFront, //glm::vec3(x, y, z),
		glm::vec3(0, 1, 0)
	);
	glm::mat4 Model = glm::mat4(1.0f);
	//Model = glm::rotate(Model, glm::radians(30.0f), glm::vec3(x, y, z));
	glm::mat4 mvp = Project * View * Model;

	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}

void generateCubePoints()
{
	cube = new CubePoints(imageResDepth);
	VBO = cube->m_PositionVBOID;
	VAO = cube->m_PositionVAOID;
	pointCount = cube->m_NumVertices;
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

int row = 0;
int col = 0;
int faceCurDepth = 0;

void flipDebug()
{
	GLuint uDebug = glGetUniformLocation(program, "Debug");
	glUniform1f(uDebug, DEBUG);
}

void threadnextface(int face)
{
	int faceCurDepth = cube->FaceCurrentDepth(face);
	ImageHandler::LoadFaceImage(face, faceCurDepth + 1);
	//cube->FaceNextDepth(face);
}

void LoadNextFaceDepth(int face)
{
	int faceCurDepth = cube->FaceCurrentDepth(face);
	if (faceCurDepth < imageResDepth) {
		std::thread t(threadnextface, face);
		t.detach();
		//std::thread t(ImageHandler::LoadFaceImage, face, faceCurDepth + 1);
		//t.detach();
		////ImageHandler::LoadFaceImage( face, faceCurDepth + 1);
		////std::thread t2(cube->FaceNextDepth, face);
		//std::thread t2 = cube->FaceNextDepthThread(face);
		//t2.detach();
	}
	else {
		fprintf(stderr, "Face %d is already at max depth\n", face);
	}
}

void threadnextquad(int face, int row, int col)
{
	int quadCurDepth = cube->QuadCurrentDepth(face, row, col);
	ImageHandler::LoadQuadImage(face, row, col, quadCurDepth + 1);
	//cube->QuadNextDepth(face, row, col);
}

void LoadNextQuadDepth(int face, int row, int col)
{
	int quadCurDepth = cube->QuadCurrentDepth(face, row, col);
	if (quadCurDepth < imageResDepth) {
		std::thread t(threadnextquad, face, row, col);
		t.detach();
		//ImageHandler::LoadQuadImage(face, row, col, quadCurDepth + 1);
		//std::thread t(ImageHandler::LoadQuadImage, face, row, col, quadCurDepth + 1);
		//t.detach();
		//cube->QuadNextDepth(face, row, col);
		//std::thread t2 = cube->QuadNextDepthThread(face, row, col);
		//t2.detach();
	}
	else {
		fprintf(stderr, "Quad %d/%d is already at max depth\n", row, col);
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
	// Zoom doesn't really work right at the moment, and so the matrix does not use it
	if (dir > 0)
	{
		zoom += 0.1f;
	}
	else
	{
		zoom -= 0.1f;
	}

	updatematrix();
}

int testdepth = 1;
void testloop()
{
	int stepping = 8 / pow(2, testdepth);
	//for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 8; j += stepping) {
			for (int k = 0; k < 8; k += stepping) {
				LoadNextQuadDepth(0, j, k);
			}
		}
		testdepth++;
	//}
}

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
		if (row >= 7) {
			row = 0;
		}
		else {
			row++;
		}
		fprintf(stderr, "Select row/col to increase depth: %d / %d\n", row, col);
		//imageResDepth++;
		//generateCubePoints();
		break;

	case GLUT_KEY_PAGE_DOWN:
		if (col >= 7) {
			col = 0;
		}
		else {
			col++;
		}
		fprintf(stderr, "Select row/col to increase depth: %d / %d\n", row, col);
		//if (imageResDepth > 0) {
		//	imageResDepth--;
		//	generateCubePoints();
		//}
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

	case GLUT_KEY_F8:
		if (faceCurDepth < 3) {
			LoadNextFaceDepth(0);
		}
		faceCurDepth++;
		break;

	case GLUT_KEY_F9:
		LoadNextQuadDepth(0, row, col);
		break;

	case GLUT_KEY_F10:
		DEBUG = !DEBUG;
		flipDebug();
		break;

	case GLUT_KEY_F11:
		std::thread t(testloop);
		t.detach();
		//testloop();
		break;
	}
	updatematrix();
}

void processKeys(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':
		LoadNextFaceDepth(0);
		break;

	case '2':
		LoadNextFaceDepth(1);
		break;

	case '3':
		LoadNextFaceDepth(2);
		break;

	case '4':
		LoadNextFaceDepth(3);
		break;

	case '5':
		LoadNextFaceDepth(4);
		break;

	case '6':
		LoadNextFaceDepth(5);
		break;

	case 27:
		glutLeaveFullScreen();
		break;
	}
	
}

void idleFunc(void)
{
	// This is used to make camera movement smoother, comment it out if it makes your dev machine laggy
	if (!ImageQueue::IsEmpty()) {
		ImageData image = ImageQueue::Dequeue();
		ImageHandler::LoadImageData(image);
		//cube->QuadNextDepth(image.face, image.row, image.col);
		std::thread t2 = cube->QuadNextDepthThread(image.face, image.row, image.col);
		t2.detach();
	}

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
	glutInitWindowSize(width, height);
	//glutInitWindowPosition(0, 0);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE);

	/* create and set up a window */
	glutCreateWindow("Test Triangles");

	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutSpecialFunc(processGlutKeys);
	glutKeyboardFunc(processKeys);
	//glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	glutMouseWheelFunc(mouseWheel);
	//glutFullScreen();

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);
	// Not 100% sure this is actually doing anything
	glDisable(GL_CULL_FACE);

	program = ShaderHelper::CreateProgram();

	if (argc >= 2) {
		ImageHandler::InitPanoListFromOnlineFile(argv[1]);
	}
	ImageHandler::InitTextureAtlas(program);
	//ImageHandler::LoadFaceImage(0, 0);

	while (!ImageQueue::IsEmpty()) {
		ImageData image = ImageQueue::Dequeue();
		ImageHandler::LoadImageData(image);
	}


	generateCubePoints();
	updatematrix();
	//GLuint uDebug = glGetUniformLocation(program, "Debug");
	//if (uDebug == -1) {
	//	fprintf(stderr, "Could not find Debug uniform\n");
	//}
	//glUniform1f(uDebug, DEBUG);

	/* define the projection transformation */
	//perspective(45, width/height, 0.1f, 100.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, width / height, 0.1f, 100.0f);
	glViewport(0, 0, width, height);
	//gluLookAt(0.0f, 0.0f, -10.f,
	//	0.0f, 0.0f, -1.0f,
	//	0.0f, 1.0f, 0.0f);
	/* tell GLUT to wait for events */
	fprintf(stderr, "You should be seeing something on the screen now.\n");
	glutMainLoop();
}
