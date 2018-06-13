#ifndef _SHARED_H
#define _SHARED_H

#include <GL\glew.h>

#define DEBUG // Comment out to disable debug macro blocks for all files importing Shared.h
#define OCULUS


#define PRINT_GL_ERRORS \
{\
	GLenum errCode;\
	if ((errCode = glGetError()) != GL_NO_ERROR) {\
		const GLubyte *errString = gluErrorString(errCode);\
		printf("OpenGL error on line %d in %s: %s\n", __LINE__, __FUNCTION__, errString);\
	}\
}

extern bool DEBUG_FLAG;

#endif // _SHARED_H