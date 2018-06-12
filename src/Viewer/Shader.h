#ifndef SHADERHELPER_H
#define SHADERHELPER_H

#include <GL\glew.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "glm/glm.hpp"

class Shader {
public:
	void createProgram(char* geometryFile, char* vertexFile, char* fragmentFile);
	void reload();
	void bind();
	void setMatrixUniform(const char* nameInShader, glm::mat4x4 matrix);

	GLuint getProgram();
	
	Shader() {}

private:
	std::string readFile(const char *filePath);
	GLuint createShader(GLenum type, const GLchar *src);
	GLuint loadShader(GLenum type, const char *path);
	void reloadShader(GLenum type, GLuint &shader, const char *path);

	GLuint m_program;
	GLuint m_geomShader;
	GLuint m_vertShader;
	GLuint m_fragShader;
};

#endif