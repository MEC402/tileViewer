// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include <vector>
#include <time.h>
#include <stdlib.h>


//TODO: These need to be yanked out into their own files to keep things tidy, and so later we can swap
// shaders out more easily (and also we won't have to recompile the whole program if we modify them)
// They live here for now because Visual Studio wouldn't stop complaining about them.
const char* fragSrc = R"glsl(
	#version 330 core

	in vec3 fColor;
	out vec4 outColor;

	void main() 
	{
		outColor = vec4(fColor, 1.0);
	}

)glsl";

const char* vertSrc = R"glsl(
	#version 330 core

	in vec3 pos;
	in vec3 color;
	in vec3 offset;

	out vec3 vColor;
	out vec3 vOffset;

	uniform float RotationAngle;

	mat4 rotationMatrix(vec3 axis, float angle) {
	    axis = normalize(axis);
	    float s = sin(angle);
	    float c = cos(angle);
	    float oc = 1.0 - c;
	    
	    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
	                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
	                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
	                0.0,                                0.0,                                0.0,                                1.0);
	}

	void main() 
	{
		mat4 rotMat = rotationMatrix(vec3(0.3,1.0,0.0), RotationAngle);
		vec4 newPos = vec4(pos, 1.0) * rotMat;
		vec4 newOffset = vec4(offset, 1.0) * rotMat; 
		gl_Position = newPos; //vec4(pos, 1.0);
		vColor = color;
		vOffset = newOffset.xyz;
	}

)glsl";

const char* geomSrc = R"glsl(
	#version 330 core
	
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;

	in vec3 vColor[];
	in vec3 vOffset[];

	out vec3 fColor;


	void main()
	{
		fColor = vColor[0];
		
		float x = vOffset[0].x;
		float y = vOffset[0].y;
		float z = vOffset[0].z;

		vec4 position = gl_in[0].gl_Position;

		gl_Position = position + vec4(-x, -y, z, 0.0);
        EmitVertex(); 
					  
        gl_Position = position + vec4( x, -y, z, 0.0);
        EmitVertex(); 						  
					  						  
        gl_Position = position + vec4(-x,  y, z, 0.0);
        EmitVertex(); 						  
					  						  
        gl_Position = position + vec4( x,  y, z, 0.0);
        EmitVertex();

		EndPrimitive();
	}
)glsl";

// This will identify our vertex buffer
GLuint vertexbuffer;
GLsizei vertexcount;
GLuint program;

float angle = 0.0f;
float anglerate = 0.001f;
float x = 0.0f, y = 0.0f, z = 0.0f;

//TODO: These probably shouldn't be defined here
// Double whatever our x/y Offsets are in the geom shader
float xStepping = 0.1f;
float yStepping = 0.1f;

void display()
{
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	/* clear window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	angle = angle + anglerate;
	if (angle >= 360.0f) {
		angle = 0.0f;
	}

	GLuint uRotationAngle = glGetUniformLocation(program, "RotationAngle");
	if (uRotationAngle != -1) {
		glUniform1f(uRotationAngle, angle);
	}
	
	glBindVertexArray(vertexbuffer);
	
	//glTranslatef(-1.0f, -y, -z);
	//glRotatef(angle, angle, angle, 1.0f);
	//glTranslatef(1.0f, -y, -z);

	glDrawArrays(GL_POINTS, 0, vertexcount);
	
	//// draw scene
	//// 1rst attribute buffer : vertices
	//glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	//glVertexAttribPointer(
	//	0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
	//	3,                  // size
	//	GL_FLOAT,           // type
	//	GL_FALSE,           // normalized?
	//	0,                  // stride
	//	(void*)0            // array buffer offset
	//);
	//angle = angle + anglerate;
	//if (angle >= 360.0f) {
	//	angle = 0.0f;
	//}
	//glTranslatef(-x, -y, -z);
	//glRotatef(angle, angle, angle, 1.0f);
	//

	//// Draw a triangle
	//glDrawArrays(GL_TRIANGLES, 0, vertexcount); // Starting from vertex 0; 3 vertices total -> 1 triangle
	//glDisableVertexAttribArray(0);
	//
	/* flush drawing routines to the window */
	glFlush();
}

GLuint createShader(GLenum type, const GLchar* src) 
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	return shader;
}

void createProgram()
{
	GLuint vertShader = createShader(GL_VERTEX_SHADER, vertSrc);
	GLuint geomShader = createShader(GL_GEOMETRY_SHADER, geomSrc);
	GLuint fragShader = createShader(GL_FRAGMENT_SHADER, fragSrc);

	program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, geomShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glUseProgram(program);
	//return program;
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

void processKeys(int key, int x1, int y1)
{
	switch (key) {
	case GLUT_KEY_UP:
		z += 0.05f;
		break;

	case GLUT_KEY_DOWN:
		z -= 0.05f;
		break;

	case GLUT_KEY_RIGHT:
		x += 0.05f;
		break;

	case GLUT_KEY_LEFT:
		x -= 0.05f;
		break;

	case GLUT_KEY_PAGE_UP:
		anglerate += 0.05f;
		break;

	case GLUT_KEY_PAGE_DOWN:
		anglerate -= 0.05f;
		break;
	}
	//OutputDebugString(_T("xyz is now: %d %d %y", x, y, z));
}

void escape(unsigned char key, int x, int y)
{
	glutLeaveFullScreen();
}

// Return an array of 2D points with random color data to test the geometry shader
GLuint generate2DPointField(int numpoints) {
	srand(time(NULL));

	if (numpoints % 2 == 0) {
		numpoints += 1;
	}

	std::vector<float> points(numpoints * 5);
	float xOffset = 0.0f;
	float yOffset = 0.0f;
	float rOffset = 0.0f;
	float gOffset = 0.0f;
	float bOffset = 0.0f;
	int perRow = sqrt(numpoints) * 5;
	for (int i = 0; i < 5 * numpoints;) {
		if (i != 0 && i % perRow == 0) {
			xOffset = 0.0f;
			yOffset -= 0.4f;
		}
		points[i++] = -0.5f + xOffset;
		points[i++] = 0.5f + yOffset;
		points[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		points[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		points[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		xOffset += 0.4f;
	}
	
	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glBindVertexArray(0);

	return VAO;
}

struct pointField3D {
	GLuint vertObj;
	GLuint pointCount;
};

// Do the same thing as 2Dpointfield but add an extra axis and put in logic to mimic Cube.cpp
// TODO: Need to properly arrange coordinates to form a cube of points
pointField3D generate3DPointField(int maxResDepth)
{
	srand(time(NULL));

	int datasize = 9;

	int faceDimensions = maxResDepth + 1;
	int faceQuads = faceDimensions * faceDimensions;
	std::vector<float> pointdata(6 * faceQuads * datasize); // 6 faces, 9 data points (xyz/rgb/geom data), quads per face

	int perRow = faceDimensions * datasize;
	
	for (int face = 0; face < 3; ++face) {
		int faceBegin = (faceQuads * datasize) * face;
		int faceEnd = (faceQuads * datasize) * (face + 1);

		float quadX = -0.5f;
		float quadY = 0.5f;
		float xOffset = 0.0f;
		float yOffset = 0.0f;

		// Point positional data
		float x = 0.0f; 
		float y = 0.0f;
		float z = 0.0f;

		// Geometry "build quad on these points" data
		float g_x = 0.0f;
		float g_y = 0.0f;
		float g_z = 0.0f;

		for (int quadPoint = faceBegin; quadPoint < faceEnd;) {
			
			switch (face) {
			case 0: // Front face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = -0.5f;
				g_x = 0.05f;
				g_y = 0.05f;
				g_z = 0.00f;
				break;
			case 1: // Back face
				x = quadX + xOffset; // -0.3f; // Debug value to make sure we're actually drawing these guys (we are)
				y = quadY + yOffset;
				z = 0.5f;
				g_x = 0.05f;
				g_y = 0.05f;
				g_z = 0.00f;
				break;
			case 2: // Right face
				x = 0.5f;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f; //0.10f;
				g_y = 0.05f;
				g_z = 0.05f;
				break;
			case 3: // Left face
				x = -0.5f;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f;
				g_y = 0.05f;
				g_z = 0.05f;
				break;
			case 4: // Top face
				x = quadX + xOffset;
				y = 0.05f;
				z = quadY + yOffset;
				g_x = 0.05f;
				g_y = 0.00f;
				g_z = 0.05f;
				break;
			case 5:
				x = quadX + xOffset;
				y = -0.05f;
				z = quadY + yOffset;
				g_x = 0.05f;
				g_y = 0.00f;
				g_z = 0.05f;
				break;
			}

			pointdata[quadPoint++] = x;
			pointdata[quadPoint++] = y;
			pointdata[quadPoint++] = z;
			pointdata[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			pointdata[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			pointdata[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			pointdata[quadPoint++] = g_x;
			pointdata[quadPoint++] = g_y;
			pointdata[quadPoint++] = g_z;

			xOffset += xStepping;
			if (quadPoint != 0 && quadPoint % perRow == 0) {
				xOffset = 0.0f;
				yOffset -= yStepping;
			}
		}
	}

	//for (int i = 0; i < pointdata.size();) {
	//	if (i != 0 && i % perRow == 0) {
	//		xOffset = 0.0f;
	//		yOffset -= yStepping;
	//		zOffset -= xStepping;
	//	}
	//	pointdata[i++] = -0.5f + xOffset;
	//	pointdata[i++] = 0.5f + yOffset;
	//	pointdata[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	pointdata[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	pointdata[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	pointdata[i++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
	//	pointdata[i++] = 0.05f;
	//	pointdata[i++] = 0.05f;
	//	pointdata[i++] = 0.00f;
	//	xOffset += xStepping;
	//}


	unsigned int VBO, VAO;
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, pointdata.size() * sizeof(float), &pointdata.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, datasize * sizeof(float), 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, datasize * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, datasize * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	return pointField3D{ (GLuint)VAO, (GLuint)(6 * faceQuads) };
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

	//Cube cube(0);
	//vertexbuffer = cube.m_PositionVBOID;
	//vertexcount = cube.m_NumVertices;
	
	createProgram();	
	


	//vertexbuffer = generate2DPointField(8);

	pointField3D temp = generate3DPointField(2);
	vertexbuffer = temp.vertObj;
	vertexcount = temp.pointCount;

	/* define the projection transformation */
	//perspective(45, width/height, 0.1f, 100.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, width/height, 0.1f, 100.0f);

	gluLookAt(0.0f, 0.0f, -10.0f, // Camera Position
		0.0f, 0.0f, -1.0f, // Looking at coords
		0.0f, 1.0f, 0.0f); // Up direction

	/* tell GLUT to wait for events */
	glutMainLoop();
}
