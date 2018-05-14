#include "SpriteBatch.h"
#include "Error.h"
#include "GLM/gtx/vector_angle.hpp"

SpriteBatch::SpriteBatch(bool dynamic) : m_dynamic(dynamic) {
	TRY_GL(glGenVertexArrays(1, &m_vertexArrayObjectID));
	TRY_GL(glBindVertexArray(m_vertexArrayObjectID));
	TRY_GL(glGenBuffers(1, &m_vertexBufferObjectID));
	TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjectID));
	TRY_GL(glEnableVertexAttribArray(0));
	TRY_GL(glEnableVertexAttribArray(1));
	TRY_GL(glEnableVertexAttribArray(2));
	TRY_GL(glEnableVertexAttribArray(3));
	TRY_GL(glEnableVertexAttribArray(4));
	TRY_GL(glEnableVertexAttribArray(5));
	TRY_GL(glEnableVertexAttribArray(6));
	TRY_GL(glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, position)
	));
	TRY_GL(glVertexAttribPointer(
		1, 2, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, dimensions)
	));
	TRY_GL(glVertexAttribPointer(
		2, 2, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, texturePosition)
	));
	TRY_GL(glVertexAttribPointer(
		3, 2, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, textureDimensions)
	));
	TRY_GL(glVertexAttribPointer(
		4, 2, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, rotationAxis)
	));
	TRY_GL(glVertexAttribPointer(
		5, 1, GL_FLOAT, GL_FALSE, sizeof(Canvas), (void*)offsetof(Canvas, rotationAngle)
	));
	TRY_GL(glVertexAttribPointer(
		6, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Canvas), (void*)offsetof(Canvas, color)
	));
	TRY_GL(glBindVertexArray(0));
	TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
int SpriteBatch::makeSprite(const Texture& texture) {
	if (!m_dynamic) { m_bufferStatic = true; }
	if (!m_deadBufferIndices.empty()) {
		auto& sprite = m_spriteBuffer[m_deadBufferIndices.back()];
		sprite = Sprite();
		sprite.texture = texture;
		int spriteID = m_deadBufferIndices.back() + 1;
		m_deadBufferIndices.pop_back();
		return spriteID;
	} else {
		m_spriteBuffer.emplace_back();
		m_spriteBuffer.back().texture = texture;
		return m_spriteBuffer.size();
	}
}
SpriteBatch::Sprite& SpriteBatch::operator [] (int spriteID) {
	if (!m_dynamic) {
		m_bufferStatic = true;
	}
	return m_spriteBuffer[spriteID - 1];
}
void SpriteBatch::destroySprite(int spriteID) {
	if (!m_dynamic) { m_bufferStatic = true; }
	int spriteIndex = spriteID - 1;
	m_deadBufferIndices.push_back(spriteIndex);
	m_spriteBuffer[spriteIndex].canvas.color.a = 0;
}
void SpriteBatch::m_makeSpriteTrays() {
	m_spriteTrays.clear();
	// Filter out dead sprites
	m_spritePtrs.clear();
	for (auto&& sprite : m_spriteBuffer) {
		if (sprite.canvas.color.a != 0) { m_spritePtrs.push_back(&sprite); }
	}
	std::sort(
		m_spritePtrs.begin(),
		m_spritePtrs.end(),
		[](Sprite* aPtr, Sprite* bPtr) { return (
			aPtr->canvas.position.z == bPtr->canvas.position.z
			? aPtr->texture.id < bPtr->texture.id
			: aPtr->canvas.position.z > bPtr->canvas.position.z
		); }
	);
	// Make sure there are sprites to render
	if (m_spritePtrs.empty()) { return; }
	// Make new tray (emplace lets you call the constructor for the class in its parameters)
	m_spriteTrays.emplace_back(m_spritePtrs[0]->texture.id, m_spritePtrs[0]->canvas.position.z);
	int offset = 0;
	m_vertexBuffer.clear();
	// Pass in vertices of the first sprite
	m_vertexBuffer.emplace_back(m_spritePtrs[0]->canvas);
	++offset;
	// Process the rest of the sprites
	for (unsigned int currentSprite = 1; currentSprite < m_spritePtrs.size(); currentSprite++) {
		// Check if this sprite has the same texture as the last sprite
		if (
			m_spritePtrs[currentSprite]->texture.id != m_spritePtrs[currentSprite - 1]->texture.id
			|| m_spritePtrs[currentSprite]->canvas.position.z
			!= m_spritePtrs[currentSprite - 1]->canvas.position.z
		) {
			// Make new a tray and place the new texture in there
			m_spriteTrays.emplace_back(
				m_spritePtrs[currentSprite]->texture.id,
				m_spritePtrs[currentSprite]->canvas.position.z,
				offset
			);
		} else {
			// Increase number of vertices For the current tray
			++m_spriteTrays.back().size;
		}
		// Pass in vertices
		m_vertexBuffer.emplace_back(m_spritePtrs[currentSprite]->canvas);
		++offset;
	}
}
void SpriteBatch::m_render() {
	TRY_GL(glBindVertexArray(m_vertexArrayObjectID));
	if (m_dynamic) {
		m_makeSpriteTrays();
		// Buffer data in GPU.
		TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjectID));
		TRY_GL(
			glBufferData(
				GL_ARRAY_BUFFER, sizeof(Canvas) * m_vertexBuffer.size(), m_vertexBuffer.data(), GL_DYNAMIC_DRAW
			)
		);
		TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
	} else if (m_bufferStatic) {
		m_makeSpriteTrays();
		// Buffer data in GPU.
		TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, m_vertexBufferObjectID));
		TRY_GL(
			glBufferData(
				GL_ARRAY_BUFFER, sizeof(Canvas) * m_vertexBuffer.size(), m_vertexBuffer.data(), GL_STATIC_DRAW
			)
		);
		TRY_GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
		m_bufferStatic = false;
	}
	// Draw data.
	for (auto&& tray : m_spriteTrays) {
		TRY_GL(glBindTexture(GL_TEXTURE_2D, tray.textureID));
		TRY_GL(glDrawArrays(GL_POINTS, tray.offset, tray.size));
	}
	TRY_GL(glBindTexture(GL_TEXTURE_2D, 0));
	TRY_GL(glBindVertexArray(0));
}

void SpriteBatch::Sprite::move(const glm::vec2& translation) {
	canvas.position.x += translation.x;
	canvas.position.y += translation.y;
}
void SpriteBatch::Sprite::move(const float x, const float y) {
	canvas.position.x += x;
	canvas.position.y += y;
}
glm::vec2 SpriteBatch::Sprite::getPosition() const {
	return glm::vec2(canvas.position.x, canvas.position.y);
}
void SpriteBatch::Sprite::setPosition(const glm::vec2& position) {
	canvas.position.x = position.x;
	canvas.position.y = position.y;
}
void SpriteBatch::Sprite::setPosition(const float x, const float y) {
	canvas.position.x = x;
	canvas.position.y = y;
}
void SpriteBatch::Sprite::setDimensions(const float width, const float height) {
	canvas.dimensions.x = width;
	canvas.dimensions.y = height;
}
void SpriteBatch::Sprite::setRotationAxis(const float x, const float y) {
	canvas.rotationAxis.x = x;
	canvas.rotationAxis.y = y;
}
void SpriteBatch::Sprite::setColor(const char r, const char g, const char b, const char a) {
	canvas.color.r = r;
	canvas.color.g = g;
	canvas.color.b = b;
	canvas.color.a = a;
}
void SpriteBatch::Sprite::setTexturePosition(const float x, const float y) {
	canvas.texturePosition.x = x;
	canvas.texturePosition.y = y;
}
void SpriteBatch::Sprite::setTextureDimensions(const float width, const float height) {
	canvas.textureDimensions.x = width;
	canvas.textureDimensions.y = height;
}
void SpriteBatch::Sprite::setFrame(const int frame) {
	canvas.texturePosition.x = canvas.textureDimensions.x * frame;
}
void SpriteBatch::Sprite::setTexture(const Texture& texture) {
	this->texture = texture;
}
void SpriteBatch::Sprite::makeLine(glm::vec2& b, glm::vec2& a, float thickness) {
	canvas.dimensions.y = glm::distance(b, a);
	canvas.dimensions.x = thickness;
	glm::vec2 difVec{ a - b };
	canvas.position.x = b.x + difVec.x/2;
	canvas.position.y = b.y + difVec.y/2;
	canvas.rotationAxis.x = canvas.position.x;
	canvas.rotationAxis.y = canvas.position.y;
	canvas.rotationAngle = glm::orientedAngle(glm::vec2(0, 1), glm::normalize(difVec));
}

SpriteBatch::SpriteTray::SpriteTray(GLuint textureID, float depth, int offset)
	: textureID(textureID), depth(depth), offset(offset) {
}