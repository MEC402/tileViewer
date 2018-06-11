//#include "stdafx.h"
#include "Shader.h"


/* ---------------- Public Functions ---------------- */

void Shader::createProgram(char* geometryFile, char* vertexFile, char* fragmentFile)
{

	GLuint program = glCreateProgram();
	if (vertexFile) {
		m_vertShader = loadShader(GL_VERTEX_SHADER, vertexFile);
		glAttachShader(program, m_vertShader);
	}
	if (geometryFile) {
		m_geomShader = loadShader(GL_GEOMETRY_SHADER, geometryFile);
		glAttachShader(program, m_geomShader);
	}
	if (fragmentFile) {
		m_fragShader = loadShader(GL_FRAGMENT_SHADER, fragmentFile);
		glAttachShader(program, m_fragShader);
	}
	glLinkProgram(program);
	m_program = program;
}

void Shader::reload()
{
	reloadShader(GL_VERTEX_SHADER, m_vertShader, "Shader.vert");
	reloadShader(GL_GEOMETRY_SHADER, m_geomShader, "Shader.geom");
	reloadShader(GL_FRAGMENT_SHADER, m_fragShader, "Shader.frag");
}

void Shader::bind()
{
	glUseProgram(m_program);
}

GLuint Shader::getProgram()
{
	return m_program;
}

/* ---------------- Private Functions ---------------- */


void Shader::reloadShader(GLenum type, GLuint &shader, const char *path)
{
	glDetachShader(m_program, shader);
	glDeleteShader(shader);
	shader = loadShader(type, path);
	glAttachShader(m_program, shader);
	glLinkProgram(m_program);
}

GLuint Shader::loadShader(GLenum type, const char *path)
{
	std::string shaderData = readFile(path);
	if (shaderData == "") {
		fprintf(stderr, "Could not loader shader.");
		return -1;
	}
	const char *shaderSrc = shaderData.c_str();
	return createShader(type, shaderSrc);
}

std::string Shader::readFile(const char *filePath)
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

GLuint Shader::createShader(GLenum type, const GLchar *src)
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
