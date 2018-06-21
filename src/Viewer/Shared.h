#ifndef _SHARED_H
#define _SHARED_H

#include <GL\glew.h>
#include <stdio.h>

#define DEBUG // Comment out to disable debug macro blocks for all files importing Shared.h
//#define KINECT
#define OCULUS


#define PRINT_GL_ERRORS print_gl_errors(__LINE__, __FUNCTION__);

inline void print_gl_errors(int line, const char* func)
{
	GLenum errCode;
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		const GLubyte *errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL error on line %d in %s: %s\n", line, func, errString);
	}
}

extern bool DEBUG_FLAG;

#endif // _SHARED_H