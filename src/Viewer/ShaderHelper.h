#pragma once
#include <GL\glew.h>
#include <fstream>
#include <iostream>
#include <string>

class ShaderHelper {
public:
	static GLuint CreateProgram();
	static GLuint ReloadShader(GLenum type);

private:
	static std::string readFile(const char *filePath);
	static GLuint createShader(GLenum type, const GLchar *src);
	static GLuint loadShader(GLenum type, const char *path);
	static GLuint reloadShader(GLenum type, GLuint &shader, const char *path);

	static GLuint m_program;
	static GLuint m_vertShader;
	static GLuint m_geomShader;
	static GLuint m_fragShader;
};