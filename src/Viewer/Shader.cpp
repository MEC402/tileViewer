//#include "stdafx.h"
#include "Shader.h"
#include "Shared.h"


/* ---------------- Public Functions ---------------- */

void Shader::CreateProgram(const char* geometryFile, const char* vertexFile, const char* fragmentFile)
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

void Shader::Reload()
{
	reloadShader(GL_VERTEX_SHADER, m_vertShader, "Shader.vert");
	reloadShader(GL_GEOMETRY_SHADER, m_geomShader, "Shader.geom");
	reloadShader(GL_FRAGMENT_SHADER, m_fragShader, "Shader.frag");
}

void Shader::Bind()
{
	glUseProgram(m_program);
	PRINT_GL_ERRORS
}

void Shader::ToggleDebug()
{
	GLuint uDebug = glGetUniformLocation(m_program, "Debug");
	glUniform1f(uDebug, DEBUG_FLAG);
}

void Shader::SetFloatUniform(const char* nameInShader, float value)
{
	GLuint uniform = glGetUniformLocation(m_program, nameInShader);
	if (uniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", nameInShader);
	}
	else {
		glUniform1f(uniform, value);
	}
}

void Shader::SetMatrixUniform(const char* nameInShader, glm::mat4x4 matrix)
{
	GLuint MatrixID = glGetUniformLocation(m_program, "MVP");
	if (MatrixID == -1) {
		fprintf(stderr, "Error getting %s uniform\n", nameInShader);
	}
	else {
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, (float*)&(matrix));
	}
}

void Shader::SetSamplerUniform(const char* nameInShader, int textureSlot)
{
	GLuint uniform = glGetUniformLocation(m_program, nameInShader);
	if (uniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", nameInShader);
	}
	else {
		glUniform1i(uniform, textureSlot);
	}
}

void Shader::BindTexture(const char* samplerNameInShader, int activeTextureSlot, GLuint textureID)
{
	GLuint TxUniform = glGetUniformLocation(m_program, samplerNameInShader);
	if (TxUniform == -1) {
		fprintf(stderr, "Error getting %s uniform\n", samplerNameInShader);
	}
	else {
		glActiveTexture(GL_TEXTURE0 + activeTextureSlot);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glUniform1i(TxUniform, activeTextureSlot);
	}
}

GLuint Shader::GetProgram()
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
