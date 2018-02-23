#pragma once
#include <map>
#include <GL\glew.h>


template<class T>
class FileCache {

public:

	/* Get date (T) from the given file path.
	If the data is not already in memory it loads it in.
	@param FILE_PATH The file path to the data (T).
	@return The data (T).
	*/
	virtual T get(const std::string& FILE_PATH) final;

protected:

	/* How to load the data (T) into memory.
	@param FILE_PATH The file path to the data (T).
	@return The data (T).
	*/
	virtual T load(const std::string& FILE_PATH) = 0;

private:

	// A map of all the loaded data.
	std::map<std::string, T> m_map;
};



/* Contains an ID and dimentions.
*/
struct Texture {

	GLuint id{ 0 };
	int width{ 0 }, height{ 0 };

	Texture() = default;

	Texture(GLuint id) : id(id) {};

	operator GLuint() const { return id; }
};



/* Loads and stores PNG files as textures in GPU memory.
*/
class TextureCache : public FileCache<Texture> {

protected:

	/* Get a texture from the given PNG file path.
	If the texture is not already in memory it loads it into it.
	@param FILE_PATH The file path to the texture.
	@return The texture.
	*/
	Texture load(const std::string& FILE_PATH) override;
};