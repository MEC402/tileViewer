#pragma once
#include <GLEW/glew.h>


/* Handles the compilation, linking, and usage of a GLSL shader program.
^ http://www.opengl.org/wiki/Shader_Compilation */
struct Shader {
	// The type of shader.
	enum Type { VERT, FRAG, GEOM, TESC, TESE, COMP };
	// GL shader ID.
	GLuint id{ 0 };
	// The type of shader.
	Type type;
};