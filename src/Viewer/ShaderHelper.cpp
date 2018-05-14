#include "stdafx.h"
GLuint ShaderHelper::CreateProgram()
{
	GLuint vertShader = loadShader(GL_VERTEX_SHADER, "Shader.vert");
	GLuint geomShader = loadShader(GL_GEOMETRY_SHADER, "Shader.geom");
	GLuint fragShader = loadShader(GL_FRAGMENT_SHADER, "Shader.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, geomShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);
	glUseProgram(program);
	return program;
}

std::string ShaderHelper::readFile(const char *filePath)
{
	std::string content;
	std::ifstream fileStream(filePath, std::ios::in);

	if (!fileStream.is_open()) {
		std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
		return "";
	}

	std::string line = "";
	while (!fileStream.eof()) {
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;
}

GLuint ShaderHelper::createShader(GLenum type, const GLchar* src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	return shader;
}

GLuint ShaderHelper::loadShader(GLenum type, const char* path)
{
	std::string shaderData = readFile(path);
	if (shaderData == "") {
		fprintf(stderr, "Could not loader shader, terminating program.");
		return -1;
	}
	const char *shaderSrc = shaderData.c_str();
	return createShader(type, shaderSrc);
}