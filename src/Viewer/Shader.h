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
	void CreateProgram(const char* geometryFile, const char* vertexFile, const char* fragmentFile);
	void FlipDebug();
	void Reload();
	void Bind();
	void SetFloatUniform(const char* nameInShader, float value);
	void SetMatrixUniform(const char* nameInShader, glm::mat4x4 matrix);
	void SetSamplerUniform(const char* nameInShader, int textureSlot);
	void BindTexture(const char* samplerNameInShader, int activeTextureSlot, GLuint textureID);

	GLuint GetProgram();
	
	Shader() : m_program(0), m_geomShader(0), m_vertShader(0), m_fragShader(0) {}

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