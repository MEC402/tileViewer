#pragma once
#include <GLEW\glew.h>
#include <SDL\SDL.h>
#include <iostream>
#define LOG_LINE(message) std::cout << message << "\n " << __FILE__ << ":" << __LINE__ << "\n";
#ifdef _DEBUG
	// Runs block and if it causes an OpenGL error it breaks after printing some information.
	#define TRY_GL(block) block; if (checkGLError(#block, __FILE__, __LINE__)) {\
		 __debugbreak(); clearGLErrors();\
	}
	// Runs block and if it causes an SDL error it breaks after printing some information.
	#define TRY_SDL(block) block; if (checkSDLError(#block, __FILE__, __LINE__)) {\
		__debugbreak(); clearSDLErrors();\
	}
	// Checks if the given GLEW function returns an error. Note that func must eval a GLEW func return value.
	#define TRY_GLEW(func) if (checkGLEWError((func), #func, __FILE__, __LINE__)) {\
		__debugbreak();\
	}
	// Breaks if the assertion fails.
	#define BREAK_IF(condition) if (condition) {\
		__debugbreak();\
	}
#else
	#define TRY_GL(block) block;
	#define TRY_SDL(block) block;
	#define TRY_GLEW(func) func;
	#define BREAK_IF(condition)
#endif
namespace fk {


static void clearGLErrors() {
	while (glGetError() != GL_NO_ERROR);
}
static bool checkGLError(const char* function, const char* file, int line) {
	bool returnVal = false;
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << ") with function:\n"
			<< function << "\n"
			<< "in " << file << "\n"
			<< "at " << line << "\n\n";
		returnVal = true;
	}
	return returnVal;
}
static void clearSDLErrors() {
	while (SDL_GetError() != "") { SDL_ClearError(); }
}
static bool checkSDLError(const char* function, const char* file, int line) {
	bool returnVal = false;
	std::string error = SDL_GetError();
	if (error != "") {
		std::cout << "[SDL Error] (" << error.c_str() << ") with function:\n"
			<< function << "\n"
			<< "in " << file << "\n"
			<< "at " << line << "\n\n";
		returnVal = true;
	}
	return returnVal;
}
static bool checkGLEWError(unsigned int errorCode, const char* function, const char* file, int line) {
	bool returnVal = false;
	if (errorCode != GLEW_OK) {
		std::cout << "[GLEW Error] (" << glewGetErrorString(errorCode) << ") with function:\n"
			<< function << "\n"
			<< "in " << file << "\n"
			<< "at " << line << "\n\n";
		returnVal = true;
	}
	return returnVal;
}

}