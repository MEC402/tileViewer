#pragma once
#include <GLEW/glew.h>
namespace fk {


	/* Contains an ID and dimensions */
	struct Texture {
		Texture() = default;
		Texture(GLuint id) : id(id) {};
		// GL texture ID.
		GLuint id{ 0 };
		// Resolution of texture.
		int width{ 0 }, height{ 0 };
		// Number of animation frames.
		int frames{ 1 };
		operator GLuint() { return id; };
	};

}