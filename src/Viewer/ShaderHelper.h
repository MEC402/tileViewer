#pragma once
#include <GL\glew.h>
#include <fstream>
#include <iostream>
#include <string>

class ShaderHelper {
public:
	static GLuint CreateProgram();

private:
	static std::string readFile(const char *filePath);
	static GLuint createShader(GLenum type, const GLchar *src);
	static GLuint loadShader(GLenum type, const char *path);
};