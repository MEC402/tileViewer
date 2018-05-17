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
		fprintf(stderr, "Could not read file %s. File does not exist.", filePath);
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
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		fprintf(stderr, "Error compiling shader: %d", type);
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		for (int i = 0; i < errorLog.size(); i++)
			fprintf(stderr, "%c", errorLog[i]);
		fprintf(stderr, "\n");
		// Provide the infolog in whatever manor you deem best.
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.
		return NULL;
	}
	return shader;
}

GLuint ShaderHelper::loadShader(GLenum type, const char* path)
{
	std::string shaderData = readFile(path);
	if (shaderData == "") {
		fprintf(stderr, "Could not loader shader.");
		return -1;
	}
	const char *shaderSrc = shaderData.c_str();
	return createShader(type, shaderSrc);
}