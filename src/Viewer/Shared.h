#ifndef _SHARED_H
#define _SHARED_H

#include <GL\glew.h>
#include <stdio.h>

#define DEBUG // Comment out to disable debug macro blocks for all files importing Shared.h
//#define KINECT
//#define OCULUS

// The first 12 active textures are used by CubeMap faces
// This is set as a macro for ease of refactoring later, should we want to use more slots later
#define THUMB_TX_SLOT 13 
#define LEFT_EYE 0
#define RIGHT_EYE 1

#define PRINT_GL_ERRORS print_gl_errors(__LINE__, __FUNCTION__);
#define PRINT_GENERIC_ERROR(ERR) print_generic_error(__LINE__, __FUNCTION__, ERR);

inline void print_gl_errors(int line, const char *func)
{
	GLenum errCode;
	if ((errCode = glGetError()) != GL_NO_ERROR) {
		const GLubyte *errString = gluErrorString(errCode);
		fprintf(stderr, "OpenGL error on line %d in %s: %s\n", line, func, errString);
	}
}

inline void print_generic_error(int line, const char *func, char *err)
{
	fprintf(stderr, "Generic error on line %d in %s: %s\n", line, func, err);
}

extern bool DEBUG_FLAG;

#endif // _SHARED_H