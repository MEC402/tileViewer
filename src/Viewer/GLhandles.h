#ifndef GLHANDLES_H
#define GLHANDLES_H

#include <GL\freeglut.h>

// Declared externally in Viewer.cpp, but shared so we can make OpenGL calls properly in other classes like Camera.cpp
extern GLuint program;
extern GLuint VAO;
extern GLuint VBO;
extern GLsizei pointCount;

#endif