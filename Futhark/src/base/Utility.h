#pragma once
#include <random>
#include <ctime>
#include <glm/glm.hpp>
#include <GLEW/glew.h>
namespace fk {


	/* For managing the general state of the app */
	enum class GameState { PLAY, EXIT };

	/* Random number generator. */
	class Random {
	public:
		/* Get a random int.
		(min) Minimum int to get back.
		(max) Maximum int to get back.
		< Random int in the given range. */
		int getInt(int min, int max) {
			return std::uniform_int_distribution<int>(min, max)(m_generator);
		}
		/* Get a random float.
		(min) Minimum float to get back.
		(max) Maximum float to get back.
		< Random float in the given range. */
		double getFloat(double min, double max) {
			return std::uniform_real_distribution<double>(min, max)(m_generator);
		}
	private:
		// The random number generator.
		std::mt19937 m_generator{ time(nullptr) };
	};


	// 2 PI, 1 Turn.
	const double TAU = 3.14159265359 * 2;
	/* Gets the angle of a vector.
	(DIRECTION) The vector to get the angle of.
	< The angle in radians. */
	inline float makeAngle(const glm::vec2& DIRECTION) {
		if (DIRECTION == glm::vec2(0, 0)) {
			return 0.0f;
		}
		else {
			float angle(acos(glm::dot(glm::vec2(1.0f, 0.0f), DIRECTION)));
			if (DIRECTION.y < 0.0f) angle = -angle;
			return angle;
		}
	}
	/* Rotates a point around the origin.
	(POINT) The point to rotate.
	(ANGLE) How much to rotate the point in radians. */
	inline glm::vec2 rotatePoint(const glm::vec2& POINT, const float ANGLE) {
		return glm::vec2(
			POINT.x * cos(ANGLE) - POINT.y * sin(ANGLE),
			POINT.x * sin(ANGLE) + POINT.y * cos(ANGLE)
		);
	}

}