#pragma once
#include <map>
#include "../out/Shader.h"
#include "../out/Texture.h"
namespace fk {


	/* Loads and stores file data in memory. */
	template <class T>
	class FileCache {
	public:
		FileCache() = default;
		/* Get data (T) from the given file path.
		If the data (T) is not already in memory it loads it in.
		(filepath) The file path to the data.
		< The data (T) associated to the file path. */
		virtual T get(const std::string& filePath) final {
			// Look for data
			auto mapSelection = m_map.find(filePath);
			if (mapSelection != m_map.end()) {
				// If data is found return it
				return mapSelection->second;
			}
			else {
				// If data is not found then load a new data and put the data into the cache
				m_map[filePath] = p_load(filePath);
				return m_map[filePath];
			}
		};
	protected:
		/* How to load the data (T).
		(filePath) The file path to the data.
		< The actual data (T) at the file path. */
		virtual T p_load(const std::string& filePath) = 0;
	private:
		// A map of the loaded files.
		std::map<std::string, T> m_map;
	};


	/* Loads and stores PNG files as textures in GPU memory. */
	class TextureCache : public FileCache<Texture> {
	public:
		/* Get a texture from the given file path and assign its frame count.
		If the texture is not already in memory it loads it in.
		(filepath) The file path to the texture.
		(frames) The number of animation frames for this texture.
		< The texture associated to the file path. */
		Texture get(const std::string& filePath, int frames);
	protected:
		/* Load texture from the given PNG file path.
		(filepath) The file path to the texture.
		< The texture. */
		Texture p_load(const std::string& filePath) override;
	};


	/* Loads and stores GLSLShaders in memory. */
	class ShadersCache : public FileCache<Shader> {
	protected:
		/* Load texture from the given PNG file path.
		(filepath) The file path to the texture.
		< The texture. */
		Shader p_load(const std::string& filePath) override;
	};

}