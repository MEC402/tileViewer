#pragma once
#include <GLEW/glew.h>
#include <vector>
#include "SpriteBatch.h"
#include "../in/FileCache.h"
#include "Camera.h"
namespace fk {


	class SpriteRenderer {
	public:
		SpriteRenderer() = default;
		~SpriteRenderer() = default;
		/* Sets the shaders for this renderer.
		(shaders) The shaders to use. DO NOT USE MORE THAN ONE OF EACH TYPE!
		[t3chma] */
		void setShaders(std::vector<Shader>& shaders);
		/* Renders a sprite batch from the perspective of the given camera.
		(batch) The sprite batch to render.
		(cam) The perspective to use.
		[t3chma] */
		void render(SpriteBatch& batch, glm::mat4& perspective);
	private:
		GLuint m_id{ 0 };
		GLuint m_textureLocation{ 0 };
		GLuint m_camLocation{ 0 };
	};

}