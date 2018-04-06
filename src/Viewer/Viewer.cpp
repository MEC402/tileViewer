// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

// This will identify our vertex buffer
GLuint vertexbuffer;

void display()
{
	/* clear window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* draw scene */
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		0,                  // stride
		(void*)0            // array buffer offset
	);

	// Draw a triangle
	glDrawArrays(GL_TRIANGLES, 0, 3*2); // Starting from vertex 0; 3 vertices total -> 1 triangle
	glDisableVertexAttribArray(0);

	/* flush drawing routines to the window */
	glFlush();
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


void processKeys(int key, int x, int y)
{
	switch (key) {
	case GLUT_KEY_UP:
		break;

	case GLUT_KEY_DOWN:
		break;

	case GLUT_KEY_RIGHT:
		break;

	case GLUT_KEY_LEFT:
		break;

	case GLUT_KEY_PAGE_UP:
		break;

	case GLUT_KEY_PAGE_DOWN:
		break;
	}
}

void escape(unsigned char key, int x, int y)
{
	glutLeaveFullScreen();
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
	glutIdleFunc(display);
	glutSpecialFunc(processKeys);
	glutKeyboardFunc(escape);
	//glutFullScreen();

	GLenum initErr = glewInit();
	if (GLEW_OK != initErr) {
		fprintf(stderr, "Error %s\n", glewGetErrorString(initErr));
	}

	glEnable(GL_DEPTH_TEST);

	int tilesPerFace = 16;

	// An array of 3 vectors which represents 3 vertices
	static GLfloat g_vertex_buffer_data[16 * 2 * 3] = {
		1.0f, 1.0f, 10.0f,
		0.0f, 0.0f, 10.0f,
		1.0f,  0.0f, 10.0f,
		0.0f, 0.0f, 10.0f,
		1.0f, 1.0f, 10.0f,
		0.0f,  1.0f, 10.0f }; // 16 tiles, 2 triangles per tile, 3 vertices per triangle
	
	Cube cube(1);

	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	/* define the projection transformation */
	//perspective(45, width/height, 0.1f, 100.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, width/height, 0.1f, 100.0f);

	gluLookAt(0.0f, 0.0f, 0.0f, // Camera Position
		0.0f, 0.0f, 1.0f, // Looking at coords
		0.0f, 1.0f, 0.0f); // Up direction

	/* tell GLUT to wait for events */
	glutMainLoop();
}
