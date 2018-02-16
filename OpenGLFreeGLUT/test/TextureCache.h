#pragma once
#include <map>
#include <GL\glew.h>


/* Contains an ID and dimentions.
*/
struct Texture {

	Texture() {};

	Texture(GLuint id) : id(id) {};

	GLuint id;
	int width, height;

	operator GLuint() const { return id; }
};


/* Loads and stores PNG files as textures in memory.
*/
class TextureCache {

public:

	/* Constructor
	*/
	TextureCache();
			
	/* Destructor
	*/
	~TextureCache();

	/* Get a texture from the given PNG file path.
	If the texture is not already in memory it loads it into it.
	@param filepath The file path to the texture.
	@return The texture.
	*/
	Texture get(const std::string& filePath);
			
private:
			
	/* A map of all the loaded textures.
	*/
	std::map<std::string, Texture> m_textureMap;
};