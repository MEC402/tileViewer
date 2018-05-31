#ifndef GLHANDLES_H
#define GLHANDLES_H

#include <GL\freeglut.h>
#include "CubePoints.h"

// Declared externally in Viewer.cpp, but shared so we can make OpenGL calls properly in other classes like Camera.cpp
extern GLuint program;
extern GLuint VAO;
extern GLuint VBO;
extern GLsizei pointCount;

// TODO: Reorganize controls so we don't have to use shared headers like this
extern CubePoints *LeftEye;
extern CubePoints *RightEye;

#endif