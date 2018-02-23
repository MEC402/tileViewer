#include "TextureCache.h"
#include "IOManager.h"
#include "PicoPNG.h"
#include <iostream>


template<class T>
T FileCache<T>::get(const std::string& IMAGE_FILE_PATH) {
	
	Texture texture;

	// Look for texture
	auto mapSelection = m_map.find(IMAGE_FILE_PATH);
	if (mapSelection != m_map.end()) {
		texture = mapSelection->second;
	} else {
		// If texture is not found then load a new image
		texture = load(IMAGE_FILE_PATH);
		// Then put the texture into the cache
		m_map[IMAGE_FILE_PATH] = texture;
	}
	
	return texture;
}



Texture TextureCache::load(const std::string& IMAGE_FILE_PATH) {

	Texture texture;
	std::vector<unsigned char> binaryPNG;
	std::vector<unsigned char> decodedPNG;
	unsigned long width, height;

	// Load
	if (IOManager::readBinaryFileToBuffer(IMAGE_FILE_PATH, binaryPNG) == false) {
		printf(
			"IOManager failed to load PNG: %s\nin: %s\nline: %i",
			IMAGE_FILE_PATH, __FILE__, __LINE__
		);
	}

	// Decode
	int errorPICO = decodePNG(decodedPNG, width, height, &(binaryPNG[0]), binaryPNG.size());
	if (errorPICO != 0) {
		printf(
			"picoPNG failed to decode: %s\nin: %s\nline: %i\ncode: %i",
			IMAGE_FILE_PATH, __FILE__, __LINE__, errorPICO
		);
	}

	// Set up the texture with OpenGL.
	texture.width = width;
	texture.height = height;
	// Create a blank openGL texture.
	// https://www.opengl.org/sdk/docs/man/html/glGenTextures.xhtml
	glGenTextures(1, &texture.id);
	// Open openGL texture.
	// https://www.opengl.org/sdk/docs/man/html/glBindTexture.xhtml
	glBindTexture(GL_TEXTURE_2D, texture);
	// Specify a two-dimensional texture image.
	// https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(decodedPNG[0]));
	// Set texture parameters.
	// https://www.opengl.org/sdk/docs/man/html/glTexParameter.xhtml
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// Generate mipmaps for a specified texture object.
	// https://www.opengl.org/sdk/docs/man/html/glGenerateMipmap.xhtml
	glGenerateMipmap(GL_TEXTURE_2D);
	// Close openGL texture
	glBindTexture(GL_TEXTURE_2D, 0);

	return texture;
}