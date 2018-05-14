#include "FileCache.h"
#include "IOManager.h"
#include "../out/Error.h"
#include "PicoPNG.h"


Texture TextureCache::get(const std::string& IMAGE_FILE_PATH, int frames) {
	Texture texture = FileCache<Texture>::get(IMAGE_FILE_PATH);
	if (frames < 1) { frames = 1; }
	texture.frames = frames;
	return texture;
}
Texture TextureCache::p_load(const std::string& IMAGE_FILE_PATH) {
	Texture texture;
	std::string imageFilePath = "Textures/" + IMAGE_FILE_PATH;
	std::vector<unsigned char> binaryPNG;
	std::vector<unsigned char> decodedPNG;
	unsigned long width, height;
	// Load
	IOManager io;
	if (io.readBinaryFileToBuffer(imageFilePath, binaryPNG) == false) {
		LOG_LINE("IOManager failed to load PNG: " + imageFilePath);
		BREAK_IF(true);
	}
	// Decode
	int errorPICO = decodePNG(decodedPNG, width, height, &(binaryPNG[0]), binaryPNG.size());
	if (errorPICO != 0) { LOG_LINE("picoPNG failed to decode: " + imageFilePath); BREAK_IF(true); }
	// Make GLtexture
	texture.width = width;
	texture.height = height;
	// Create a blank openGL texture
	// https://www.opengl.org/sdk/docs/man/html/glGenTextures.xhtml
	TRY_GL(glGenTextures(1, &texture.id));
	// Open openGL texture
	// https://www.opengl.org/sdk/docs/man/html/glBindTexture.xhtml
	TRY_GL(glBindTexture(GL_TEXTURE_2D, texture.id));
	// Specify a two-dimensional texture image
	// https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
	TRY_GL(
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(decodedPNG[0]))
	);
	// Set texture parameters
	// https://www.opengl.org/sdk/docs/man/html/glTexParameter.xhtml
	TRY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	TRY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	TRY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	TRY_GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
	// Generate mipmaps for a specified texture object
	// https://www.opengl.org/sdk/docs/man/html/glGenerateMipmap.xhtml
	TRY_GL(glGenerateMipmap(GL_TEXTURE_2D));
	// Close openGL texture
	TRY_GL(glBindTexture(GL_TEXTURE_2D, 0));
	return texture;
}

Shader ShadersCache::p_load(const std::string& SHADER_FILE_PATH) {
	Shader shader;
	// Get type and make shader
	int dot = SHADER_FILE_PATH.find(".");
	char shell = SHADER_FILE_PATH[dot + 1];
	switch (shell) {
	  case 'v':
		TRY_GL(shader.id = glCreateShader(GL_VERTEX_SHADER));
		shader.type = Shader::VERT;
	  break;
	  case 'f':
		TRY_GL(shader.id = glCreateShader(GL_FRAGMENT_SHADER));
		shader.type = Shader::FRAG;
	  break;
	  case 'g':
		TRY_GL(shader.id = glCreateShader(GL_GEOMETRY_SHADER));
		shader.type = Shader::GEOM;
	  break;
	  case 't':
		TRY_GL(shader.id = glCreateShader(GL_TESS_CONTROL_SHADER));
		shader.type = Shader::TESC;
	  break;
	  case 'e':
		TRY_GL(shader.id = glCreateShader(GL_TESS_EVALUATION_SHADER));
		shader.type = Shader::TESE;
	  break;
	  case 'c':
		TRY_GL(shader.id = glCreateShader(GL_COMPUTE_SHADER));
		shader.type = Shader::COMP;
	  break;
	  default:
		BREAK_IF(true);
	  break;
	}
	if (shader.id == 0) { LOG_LINE("Failed to create shader"); }
	// Read in text file
	IOManager io;
	std::string shaderStringSource = "";
	std::string shaderFilePath = "Shaders/" + SHADER_FILE_PATH;
	if (io.readTextFileToString(shaderFilePath, shaderStringSource) == false) {
		LOG_LINE("Failed to read in " + shaderFilePath);
		BREAK_IF(true);
	}
	const char* shaderSource = shaderStringSource.c_str();
	TRY_GL(glShaderSource(shader.id, 1, &shaderSource, nullptr));
	glCompileShader(shader.id);
	// Error check
	GLint success = 0;
	glGetShaderiv(shader.id, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE) {
		GLint maxLength = 0;
		glGetShaderiv(shader.id, GL_INFO_LOG_LENGTH, &maxLength);
		// The maxLength includes the NULL character
		std::vector<char> errorLog(maxLength);
		glGetShaderInfoLog(shader.id, maxLength, &maxLength, &errorLog[0]);
		glDeleteShader(shader.id);
		shader.id = 0;
		std::printf("Failed to compile shader: %s\n%s\n", shaderFilePath.c_str(), &(errorLog[0]));
		LOG_LINE("");
		BREAK_IF(true);
	}
	return shader;
}