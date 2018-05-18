#include "Camera.h"
namespace fk {


	void Camera::setDimensions(const glm::vec2 DIMENSIONS) {
		m_screenDimensions = glm::vec2(DIMENSIONS.x, DIMENSIONS.y);
		m_lensDimensions = glm::vec2(DIMENSIONS.x, DIMENSIONS.y);
		m_baseMatrix = glm::ortho(0.0f, DIMENSIONS.x, 0.0f, DIMENSIONS.y);
		m_pendingUpdate = true;
	}
	void Camera::update() {
		// Only update if our position or scale have changed
		if (m_pendingUpdate) {
			m_transMatrix = glm::translate(
				m_baseMatrix,
				// Translation
				glm::vec3(
					-m_position.x + m_screenDimensions.x / 2,
					-m_position.y + m_screenDimensions.y / 2,
					0.0f
				)
			);
			m_transScaledMatrix = glm::scale(
				glm::mat4(1.0f),
				glm::vec3(m_zoom, m_zoom, 0.0f) // Scale
			) * m_transMatrix;
			m_pendingUpdate = false;
		}
	}
	void Camera::setPosition(const glm::vec2& POSITION) {
		m_position = POSITION;
		m_pendingUpdate = true;
	}
	void Camera::move(const glm::vec2& POSITION) {
		m_position.y += POSITION.y;
		m_position.x += POSITION.x;
		m_pendingUpdate = true;
	}
	void Camera::setZoom(float zoom) {
		if (zoom < 0) { return; }
		m_zoom = zoom;
		m_lensDimensions = m_screenDimensions / m_zoom;
		m_pendingUpdate = true;
	}
	void Camera::zoomIn(float scale) {
		m_zoom *= scale;
		m_lensDimensions = m_screenDimensions / m_zoom;
		m_pendingUpdate = true;
	}
	void Camera::zoomOut(float scale) {
		m_zoom /= scale;
		m_lensDimensions = m_screenDimensions / m_zoom;
		m_pendingUpdate = true;
	}
	glm::vec2 Camera::getPosition() const { return m_position; }
	glm::vec2 Camera::getScreenTopRight() const {
		return m_position + glm::vec2(m_screenDimensions.x / 2, m_screenDimensions.y / 2);
	}
	glm::vec2 Camera::getScreenTopLeft() const {
		return m_position + glm::vec2(-m_screenDimensions.x / 2, m_screenDimensions.y / 2);
	}
	glm::vec2 Camera::getScreenBottomLeft() const {
		return m_position + glm::vec2(-m_screenDimensions.x / 2, -m_screenDimensions.y / 2);
	}
	glm::vec2 Camera::getScreenBottomRight() const {
		return m_position + glm::vec2(m_screenDimensions.x / 2, -m_screenDimensions.y / 2);
	}
	glm::vec2 Camera::getLensTopRight() const {
		return m_position + glm::vec2(m_lensDimensions.x / 2, m_lensDimensions.y / 2);
	}
	glm::vec2 Camera::getLensTopLeft() const {
		return m_position + glm::vec2(-m_lensDimensions.x / 2, m_lensDimensions.y / 2);
	}
	glm::vec2 Camera::getLensBottomLeft() const {
		return m_position + glm::vec2(-m_lensDimensions.x / 2, -m_lensDimensions.y / 2);
	}
	glm::vec2 Camera::getLensBottomRight() const {
		return m_position + glm::vec2(m_lensDimensions.x / 2, -m_lensDimensions.y / 2);
	}
	glm::vec2 Camera::getWorldCoordinates(const glm::vec2& WINDOW_COORDINATES) const {
		glm::vec2 worldsCoordinates = WINDOW_COORDINATES;
		// Center 0
		worldsCoordinates -= glm::vec2(m_screenDimensions.x / 2, m_screenDimensions.y / 2);
		// Scale cords
		worldsCoordinates /= m_zoom;
		// Translate camera position
		worldsCoordinates += glm::vec2(m_position.x, -m_position.y);
		// Flip cords
		worldsCoordinates *= glm::vec2(1, -1);
		return worldsCoordinates;
	}
	float Camera::getZoom() const { return m_zoom; }
	glm::vec2 Camera::getScreenDimentions() const { return m_screenDimensions; }
	glm::vec2 Camera::getLensDimentions() const { return m_lensDimensions; }
	glm::mat4 Camera::getBaseMatrix() const { return m_baseMatrix; }
	glm::mat4 Camera::getTransMatrix() const { return m_transMatrix; }
	glm::mat4 Camera::getTransScaledMatrix() const { return m_transScaledMatrix; }

}