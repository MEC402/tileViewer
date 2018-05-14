#pragma once
#include <random>
#include <ctime>
#include <glm/glm.hpp>
#include <GLEW/glew.h>
#include "../base/Utility.h"


/* An X Y Z coordinate container. */
class Vec3 {
  public:
	GLfloat x{ 0 };
	GLfloat y{ 0 };
	GLfloat z{ 0 };
	Vec3() = default;
	/* Constructor
	(x) X coord.
	(y) Y coord. */
	Vec3(GLfloat x, GLfloat y, GLfloat z);
	/* Constructor
	(vec2) glm::vec2. */
	Vec3(const glm::vec3& VEC3);
	operator glm::vec3() const;
	Vec3 operator = (const Vec3& VEC3);
	Vec3 operator += (const Vec3& VEC3);
	Vec3 operator -= (const Vec3& VEC3);
	Vec3 operator *= (const Vec3& VEC3);
	Vec3 operator /= (const Vec3& VEC3);
	Vec3 operator + (const Vec3& VEC3) const;
	Vec3 operator - (const Vec3& VEC3) const;
	Vec3 operator * (const Vec3& VEC3) const;
	Vec3 operator / (const Vec3& VEC3) const;
	bool operator == (const Vec3& VEC3) const;
	bool operator != (const Vec3& VEC3) const;
	Vec3 operator += (const GLfloat& SCALER);
	Vec3 operator -= (const GLfloat& SCALER);
	Vec3 operator *= (const GLfloat& SCALER);
	Vec3 operator /= (const GLfloat& SCALER);
	Vec3 operator + (const GLfloat& SCALER) const;
	Vec3 operator - (const GLfloat& SCALER) const;
	Vec3 operator * (const GLfloat& SCALER) const;
	Vec3 operator / (const GLfloat& SCALER) const;
	enum Axis { X, Y, Z };
	/* Rotates a point around the origin.
	(ANGLE) How much to rotate the point in radians.
	(axis) Which axis to rotate around. */
	void rotate(const GLfloat ANGLE, Axis axis = Z);
};


/* An X Y coordinate container. */
class Vec2 {
  public:
	GLfloat x{ 0 };
	GLfloat y{ 0 };
	Vec2() = default;
	/* Constructor
	(x) X coord.
	(y) Y coord. */
	Vec2(GLfloat x, GLfloat y);
	/* Constructor
	(vec2) glm::vec2. */
	Vec2(const glm::vec2& VEC2);
	/* Constructor
	(vec2) glm::ivec2. */
	Vec2(const glm::ivec2& VEC2);
	operator glm::vec2() const;
	Vec2 operator = (const Vec2& VEC2);
	Vec2 operator += (const Vec2& VEC2);
	Vec2 operator -= (const Vec2& VEC2);
	Vec2 operator *= (const Vec2& VEC2);
	Vec2 operator /= (const Vec2& VEC2);
	Vec2 operator + (const Vec2& VEC2) const;
	Vec2 operator - (const Vec2& VEC2) const;
	Vec2 operator * (const Vec2& VEC2) const;
	Vec2 operator / (const Vec2& VEC2) const;
	bool operator == (const Vec2& VEC2) const;
	bool operator != (const Vec2& VEC2) const;
	Vec2 operator += (const GLfloat& SCALER);
	Vec2 operator -= (const GLfloat& SCALER);
	Vec2 operator *= (const GLfloat& SCALER);
	Vec2 operator /= (const GLfloat& SCALER);
	Vec2 operator + (const GLfloat& SCALER) const;
	Vec2 operator - (const GLfloat& SCALER) const;
	Vec2 operator * (const GLfloat& SCALER) const;
	Vec2 operator / (const GLfloat& SCALER) const;
	/* Rotates a point around the origin.
	(ANGLE) How much to rotate the point in radians. */
	void rotate(const GLfloat ANGLE);
};

	
/* A red, green, blue, alpha container. */
class Color {
  public:
	GLubyte r;
	GLubyte g;
	GLubyte b;
	GLubyte a;
	Color() = default;
	/* Constructor
	(r) Red value.
	(g) Green value.
	(b) Blue value.
	(a) Alpha value. */
	Color(GLubyte r, GLubyte g, GLubyte b, GLubyte a);
	Color operator = (const Color& COLOR);
	Color operator += (const Color& COLOR);
	Color operator -= (const Color& COLOR);
	Color operator *= (const Color& COLOR);
	Color operator /= (const Color& COLOR);
	Color operator + (const Color& COLOR) const;
	Color operator - (const Color& COLOR) const;
	Color operator * (const Color& COLOR) const;
	Color operator / (const Color& COLOR) const;
	Color operator += (const int& VALUE);
	Color operator -= (const int& VALUE);
	Color operator *= (const int& VALUE);
	Color operator /= (const int& VALUE);
	Color operator + (const int& VALUE) const;
	Color operator - (const int& VALUE) const;
	Color operator * (const int& VALUE) const;
	Color operator / (const int& VALUE) const;
	bool operator == (const Color& COLOR) const;
	bool operator != (const Color& COLOR) const;
};

// Preset colors
namespace PresetColors {
	// 255, 0, 0, 255
	const Color RED(255, 0, 0, 255);
	// 255, 127, 127, 255
	const Color RED_LIGHT(255, 127, 127, 255);
	// 128, 0, 0, 255
	const Color RED_DARK(127, 0, 0, 255);
	// 255, 127, 0, 255
	const Color ORANGE(255, 127, 0, 255);
	// 127, 63, 0, 255
	const Color BROWN(127, 63, 0, 255);
	// 255, 255, 0, 255
	const Color YELLOW(255, 255, 0, 255);
	// 127, 127, 0, 255
	const Color YELLOW_DARK(127, 127, 0, 255);
	// 255, 255, 128, 255
	const Color YELLOW_LIGHT(255, 255, 127, 255);
	// 127, 255, 0, 255
	const Color LIME(127, 255, 0, 255);
	// 0, 255, 0, 255
	const Color GREEN(0, 255, 0, 255);
	// 0, 127, 0, 255
	const Color GREEN_DARK(0, 127, 0, 255);
	// 127, 255, 127, 255
	const Color GREEN_LIGHT(127, 255, 127, 255);
	// 0, 255, 127, 255
	const Color CREAM_GREEN(0, 255, 127, 255);
	// 0, 255, 255, 255
	const Color CYAN(0, 255, 255, 255);
	// 0, 127, 127, 255
	const Color CYAN_DARK(0, 127, 127, 255);
	// 127, 255, 255, 255
	const Color CYAN_LIGHT(127, 255, 255, 255);
	// 0, 127, 255, 255
	const Color COBALT(0, 127, 255, 255);
	// 0, 0, 255, 255
	const Color BLUE(0, 0, 255, 255);
	// 0, 0, 127, 255
	const Color BLUE_DARK(0, 0, 127, 255);
	// 127, 127, 255, 255
	const Color BLUE_LIGHT(127, 127, 255, 255);
	// 127, 0, 255, 255
	const Color PURPLE(127, 0, 255, 255);
	// 255, 0, 255, 255
	const Color MAROON(255, 0, 255, 255);
	// 127, 0, 127, 255
	const Color MAROON_DARK(127, 0, 127, 255);
	// 255, 127, 255, 255
	const Color MAROON_LIGHT(255, 127, 255, 255);
	// 255, 0, 127, 255
	const Color HOT_PINK(255, 0, 127, 255);
	// 255, 255, 255, 255
	const Color WHITE(255, 255, 255, 255);
	// 127, 127, 127, 255
	const Color GREY(127, 127, 127, 255);
	// 0, 0, 0, 255
	const Color BLACK(0, 0, 0, 255);
	// 0, 0, 0, 127
	const Color BLACK_TRANSLUCENT(0, 0, 0, 127);
	// 255, 255, 255, 127
	const Color WHITE_TRANSLUCENT(255, 255, 255, 127);
	// 0, 0, 0, 0
	const Color INVISIBLE(0, 0, 0, 0);
}