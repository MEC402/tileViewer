//#include "stdafx.h"
#include "ShaderHelper.h"

GLuint ShaderHelper::m_program;
GLuint ShaderHelper::m_vertShader;
GLuint ShaderHelper::m_geomShader;
GLuint ShaderHelper::m_fragShader;

/* ---------------- Public Functions ---------------- */

GLuint ShaderHelper::CreateProgram()
{
	m_vertShader = loadShader(GL_VERTEX_SHADER, "Shader.vert");
	m_geomShader = loadShader(GL_GEOMETRY_SHADER, "Shader.geom");
	m_fragShader = loadShader(GL_FRAGMENT_SHADER, "Shader.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, m_vertShader);
	glAttachShader(program, m_geomShader);
	glAttachShader(program, m_fragShader);
	glLinkProgram(program);
	glUseProgram(program);
	m_program = program;
	return program;
}

GLuint ShaderHelper::ReloadShader(GLenum type)
{
	switch (type) {
	case GL_VERTEX_SHADER:
		return reloadShader(type, m_vertShader, "Shader.vert");
		break;
	case GL_GEOMETRY_SHADER:
		return reloadShader(type, m_geomShader, "Shader.geom");
		break;
	case GL_FRAGMENT_SHADER:
		return reloadShader(type, m_fragShader, "Shader.frag");
		break;
	default:
		fprintf(stderr, "No such shader type found\n");
		return -1;
	}
}

/* ---------------- Private Functions ---------------- */


GLuint ShaderHelper::reloadShader(GLenum type, GLuint &shader, const char *path)
{
	glDetachShader(m_program, shader);
	glDeleteShader(shader);
	shader = loadShader(type, path);
	glAttachShader(m_program, shader);
	glLinkProgram(m_program);
	glUseProgram(m_program);
	return m_program;
}

GLuint ShaderHelper::loadShader(GLenum type, const char *path)
{
	std::string shaderData = readFile(path);
	if (shaderData == "") {
		fprintf(stderr, "Could not loader shader.");
		return -1;
	}
	const char *shaderSrc = shaderData.c_str();
	return createShader(type, shaderSrc);
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

GLuint ShaderHelper::createShader(GLenum type, const GLchar *src)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &src, nullptr);
	glCompileShader(shader);
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		fprintf(stderr, "Error compiling shader: %s", src);
		GLint maxLength = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> errorLog(maxLength);
		glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);
		for (unsigned int i = 0; i < errorLog.size(); i++)
			fprintf(stderr, "%c", errorLog[i]);
		fprintf(stderr, "------------------------\n");
		// Exit with failure.
		glDeleteShader(shader); // Don't leak the shader.
		return NULL;
	}
	return shader;
}
