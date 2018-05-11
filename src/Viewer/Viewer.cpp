// Viewer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <math.h>
#include <glm/gtc/matrix_transform.hpp>
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

	void main() 
	{
		
		//vec4 newPos = vec4(pos, 1.0) * rotMat;
		//vec4 newPos = MVP * (vec4(pos, 1.0) * rotMat);
		//vec4 newOffset = vec4(offset, 1.0) * rotMat; 
		//vec4 newOffset = Scaling * (vec4(offset, 1.0) * rotMat); 
		gl_Position = vec4(pos, 1.0);
		vColor = color;
		vOffset = offset;
	}

)glsl";

const char* geomSrc = R"glsl(
	#version 330 core
	
	layout(points) in;
	layout(triangle_strip, max_vertices = 4) out;

	in vec3 vColor[];
	in vec3 vOffset[];

	out vec3 fColor;

	uniform mat4 MVP;
	uniform float Scaling;
	uniform float RotationAngle;

	// TODO: Extract this and push it through as a uniform
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
		fColor = vColor[0];
		
		mat4 rotMat = rotationMatrix(vec3(0.0,1.0,0.0), RotationAngle);

		vec4 position = MVP * (gl_in[0].gl_Position * rotMat);

		float x = vOffset[0].x;
		float y = vOffset[0].y;
		float z = vOffset[0].z;

		//TODO: Need to figure out the pos/neg for z values for top/bottom tiles
		gl_Position = position + Scaling * (rotMat*(vec4(-x, -y, z, 0.0)));
		EmitVertex(); 			 
					  			 
		gl_Position = position + Scaling * (rotMat*(vec4( x, -y, z, 0.0)));
		EmitVertex(); 			 
					  			 
		gl_Position = position + Scaling * (rotMat*(vec4(-x,  y, z, 0.0)));
		EmitVertex(); 			 
								 
		gl_Position = position + Scaling * (rotMat*(vec4( x,  y, z, 0.0)));
		EmitVertex();

		//gl_Position = position + vec4(-x, -y, z, 0.0);
        //EmitVertex(); 
		//			  
        //gl_Position = position + vec4( x, -y, z, 0.0);
        //EmitVertex(); 						  
		//			  						  
        //gl_Position = position + vec4(-x,  y, z, 0.0);
        //EmitVertex(); 						  
		//			  						  
        //gl_Position = position + vec4( x,  y, z, 0.0);
        //EmitVertex();

		EndPrimitive();
	}
)glsl";

// This will identify our vertex buffer
GLuint VBO;
GLuint VAO;
GLsizei pointCount;
GLuint program;

float angle = 0.0f;
float anglerate = 0.001f;
float scale = 2.42f; // Magic number with tilewidth 0.05f
float x = 0.0f, y = 0.0f, z = -1.0f;

void display()
{
	
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	/* clear window */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLuint uRotationAngle = glGetUniformLocation(program, "RotationAngle");
	if (uRotationAngle != -1) {
		glUniform1f(uRotationAngle, angle);
	}
	
	glBindVertexArray(VAO);
	glDrawArrays(GL_POINTS, 0, pointCount);
	
	/* flush drawing routines to the window */
	glFlush();
}
void updatematrix()
{
	glm::mat4 Project = glm::perspective(glm::radians(45.0f), (float)800 / (float)800, 0.1f, 100.0f);
	glm::mat4 View = glm::lookAt(
		glm::vec3(x, y, z),
		glm::vec3(0, 0, 1.0),
		glm::vec3(0, 1, 0)
	);
	glm::mat4 Model = glm::mat4(1.0f);
	glm::mat4 mvp = Project * View * Model;

	GLuint MatrixID = glGetUniformLocation(program, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &mvp[0][0]);
}

void updatescaling()
{
	GLuint uniformID = glGetUniformLocation(program, "Scaling");
	glUniform1f(uniformID, scale);
	fprintf(stderr, "Scale is at: %f", scale);
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
		y += 0.05f;
		break;

	case GLUT_KEY_DOWN:
		y -= 0.05f;
		break;

	case GLUT_KEY_RIGHT:
		x -= 0.05f;
		break;

	case GLUT_KEY_LEFT:
		x += 0.05f;
		break;

	case GLUT_KEY_PAGE_UP:
		z += 0.05f;
		break;

	case GLUT_KEY_PAGE_DOWN:
		z -= 0.05f;
		break;

	case GLUT_KEY_F2:
		scale += 0.01f;
		updatescaling();
		break;

	case GLUT_KEY_F3:
		scale -= 0.01f;
		updatescaling();
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
	angle = angle + anglerate;
	if (angle >= 360.0f) {
		angle = 0.0f;
	}
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

	CubePoints cube(2);
	VBO = cube.m_PositionVBOID;
	VAO = cube.m_PositionVAOID;
	pointCount = cube.m_NumVertices;

	updatematrix();
	updatescaling();

	/* define the projection transformation */
	//perspective(45, width/height, 0.1f, 100.0f);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, width/height, 0.1f, 100.0f);
	glViewport(0, 0, width, height);
	//gluLookAt(0.0f, 0.0f, -10.f,
	//	0.0f, 0.0f, -1.0f,
	//	0.0f, 1.0f, 0.0f);
	/* tell GLUT to wait for events */
	glutMainLoop();
}
