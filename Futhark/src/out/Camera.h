#pragma once
#include <glm/gtc/matrix_transform.hpp>
namespace fk {


	/* This represents a virtual camera. */
	class Camera {
	public:
		/* Sets the base screen dimensions.
		(DIMENSIONS) The camera screen dimensions in pixels. */
		void setDimensions(const glm::vec2 DIMENSIONS);
		/* Used to update the camera if it needs to.
		This should probably be called once every frame and
		is usually called in the process input method of your custom screen class.
		^ CustomScreen: p_customUpdate() */
		void update();
		void setPosition(const glm::vec2& POSITION);
		/* Translates the camera.
		(position) The translation. */
		void move(const glm::vec2& POSITION);
		void setZoom(float zoom);
		/* Zooms in the camera.
		This is the inverse of zoomOut().
		(scale) The zoom multiplier. */
		void zoomIn(float scale = 1.03);
		/* Zooms out the camera.
		This is the inverse of zoomIn().
		(scale) The zoom multiplier. */
		void zoomOut(float scale = 1.03);
		glm::vec2 getPosition() const;
		glm::vec2 getScreenTopRight() const;
		glm::vec2 getScreenTopLeft() const;
		glm::vec2 getScreenBottomRight() const;
		glm::vec2 getScreenBottomLeft() const;
		glm::vec2 getLensTopRight() const;
		glm::vec2 getLensTopLeft() const;
		glm::vec2 getLensBottomRight() const;
		glm::vec2 getLensBottomLeft() const;
		/* Translates window coordinates into world coordinates.
		(windowCoordinates) window coordinates.
		< World coordinates. */
		glm::vec2 getWorldCoordinates(const glm::vec2& WINDOW_COORDINATES) const;
		float getZoom() const;
		glm::vec2 getScreenDimentions() const;
		glm::vec2 getLensDimentions() const;
		glm::mat4 getBaseMatrix() const;
		glm::mat4 getTransMatrix() const;
		glm::mat4 getTransScaledMatrix() const;
	private:
		// Used to determine if the camera actually needs to be updated when update() is called.
		bool m_pendingUpdate{ true };
		// The zoom scale of the camera.
		float m_zoom{ 1.0f };
		// The base lense dimensions.
		glm::vec2 m_screenDimensions{ 1000.0f, 500.0f };
		// The scaled lense dimensions.
		glm::vec2 m_lensDimensions{ 1000.0f, 500.0f };
		// The focus (center) of the camera.
		glm::vec2 m_position{ 0.0f };
		// Base camera lense.
		glm::mat4 m_baseMatrix{ 1.0f };
		// Transformed camera lense.
		glm::mat4 m_transMatrix{ 1.0f };
		// Transformed and scaled camera lense.
		glm::mat4 m_transScaledMatrix{ 1.0f };
	};

}