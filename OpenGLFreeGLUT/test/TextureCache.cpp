#include "TextureCache.h"
#include "IOManager.h"
#include "PicoPNG.h"
#include <iostream>


TextureCache::TextureCache() {}

TextureCache::~TextureCache() {}

Texture TextureCache::get(const std::string& IMAGE_FILE_PATH) {
	// Look for texture
	auto mapSelection = m_textureMap.find(IMAGE_FILE_PATH);

	// If texture is not found then load a new image
	if (mapSelection == m_textureMap.end()) {
			
		// Picture data containers
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

		// Make GLtexture
		Texture newTexture = {};
		newTexture.width = width;
		newTexture.height = height;
		// Create a blank openGL texture
		// https://www.opengl.org/sdk/docs/man/html/glGenTextures.xhtml
		glGenTextures(1, &newTexture.id);
		// Open openGL texture
		// https://www.opengl.org/sdk/docs/man/html/glBindTexture.xhtml
		glBindTexture(GL_TEXTURE_2D, newTexture.id);
		// Specify a two-dimensional texture image
		// https://www.opengl.org/sdk/docs/man/html/glTexImage2D.xhtml
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &(decodedPNG[0]));
		// Set texture parameters
		// https://www.opengl.org/sdk/docs/man/html/glTexParameter.xhtml
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// Generate mipmaps for a specified texture object
		// https://www.opengl.org/sdk/docs/man/html/glGenerateMipmap.xhtml
		glGenerateMipmap(GL_TEXTURE_2D);
		// Close openGL texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// Then put the texture into the cache
		m_textureMap[IMAGE_FILE_PATH] = newTexture;

		return newTexture;
	}

	// If texture is found return it
	return mapSelection->second;
}