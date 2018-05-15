// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <glm/gtc/matrix_transform.hpp>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <vector>


double width = 800;
double height = 800;

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
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// Old floats, need to sanity check and remove these
float angle = 0.0f;
float anglerate = 0.001f;
float x = 1.0f, y = 0.0f, z = 0.0f;

int imageResDepth = 1;

void display()
{

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	/* clear window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// TODO: Should probably grab this elsewhere and store it in memory so we're not making location calls every frame
	//GLuint uRotationAngle = glGetUniformLocation(program, "RotationAngle");
	//if (uRotationAngle != -1) {
	//	glUniform1f(uRotationAngle, angle);
	//}

	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, pointCount);

	/* flush drawing routines to the window */
	glFlush();
}
void updatematrix()
{
	glm::mat4 Project = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 100.0f);


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

void updateScaling(float scale)
{
	GLuint uScaling = glGetUniformLocation(program, "Scaling");
	glUniform1f(uScaling, scale);
}

void generateCubePoints()
{
	cube = new CubePoints(imageResDepth);
	VBO = cube->m_PositionVBOID;
	VAO = cube->m_PositionVAOID;
	pointCount = cube->m_NumVertices;
	updateScaling(0.125f);
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


void mouseMove(int posx, int posy)
{
		fprintf(stderr, "Mouse movement %d , %d", posx, posy);
		if (firstMouse)
		{
			lastX = posx;
			lastY = posy;
			firstMouse = false;
		}

		float xoffset = posx - lastX;
		float yoffset = lastY - posy; // reversed since y-coordinates go from bottom to top
		lastX = posx;
		lastY = posy;

		float sensitivity = 0.01f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;
		updatematrix();
		lastX = width / 2;
		lastY = height / 2;
}

void processKeys(int key, int x1, int y1)
{
	switch (key) {
	case GLUT_KEY_UP:
		//y -= 0.05f;
		pitch += 1.0f;
		break;

	case GLUT_KEY_DOWN:
		//y += 0.05f;
		pitch -= 1.0f;
		break;

	case GLUT_KEY_RIGHT:
		//x += 0.05f;
		yaw += 1.0f;
		break;

	case GLUT_KEY_LEFT:
		//x -= 0.05f;
		yaw -= 1.0f;
		break;

	case GLUT_KEY_PAGE_UP:
		z += 0.05f;
		break;

	case GLUT_KEY_PAGE_DOWN:
		z -= 0.05f;
		break;

	case GLUT_KEY_F1:
		if (imageResDepth > 0) {
			imageResDepth--;
			generateCubePoints();
		}
		break;

	case GLUT_KEY_F2:
		imageResDepth++;
		generateCubePoints();
		break;

	case GLUT_KEY_F3:
		cube->loadTexture("container_x2.jpg");
		updateScaling(0.25f);
		break;

	case GLUT_KEY_F4:
		cube->loadTexture("container_x4.jpg");
		updateScaling(1.0f);
		break;
	}
	updatematrix();
}

void escape(unsigned char key, int x, int y)
{
	glutLeaveFullScreen();
}

void idleFunc(void)
{
	display();
}

int main(int argc, char * argv[])
{
	/* initialize GLUT, using any commandline parameters passed to the program */
	glutInit(&argc, argv);

	double width = 800;
	double height = 800;

	/* setup the size, position, and display mode for new windows */
	glutInitWindowSize(width, height);
	//glutInitWindowPosition(0, 0);
	//glutInitDisplayMode(GLUT_DEPTH | GLUT_RGB);

	/* create and set up a window */
	glutCreateWindow("Test Triangles");

	/* Assign glut framework function calls */
	glutDisplayFunc(display);
	glutIdleFunc(idleFunc);
	glutSpecialFunc(processKeys);
	glutKeyboardFunc(escape);
	//glutMouseFunc(mouseButton);
	glutMotionFunc(mouseMove);
	//glutFullScreen();

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);
	// Not 100% sure this is actually doing anything
	glDisable(GL_CULL_FACE);


	program = ShaderHelper::CreateProgram();

	generateCubePoints();
	updatematrix();

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
	glutMainLoop();
}
