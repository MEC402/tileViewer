#include "OutUtility.h"
namespace fk {


	Vec3::Vec3(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {}
	Vec3::Vec3(const glm::vec3& VEC3) : x(VEC3.x), y(VEC3.y), z(VEC3.z) {}
	Vec3::operator glm::vec3() const { return glm::vec3(x, y, z); }
	Vec3 Vec3::operator = (const Vec3& VEC3) { x = VEC3.x; y = VEC3.y; z = VEC3.z; return VEC3; }
	Vec3 Vec3::operator += (const Vec3& VEC3) { x += VEC3.x; y += VEC3.y; z += VEC3.z; return *this; }
	Vec3 Vec3::operator -= (const Vec3& VEC3) { x -= VEC3.x; y -= VEC3.y; z -= VEC3.z; return *this; }
	Vec3 Vec3::operator *= (const Vec3& VEC3) { x *= VEC3.x; y *= VEC3.y; z *= VEC3.z; return *this; }
	Vec3 Vec3::operator /= (const Vec3& VEC3) { x /= VEC3.x; y /= VEC3.y; z /= VEC3.z; return *this; }
	Vec3 Vec3::operator + (const Vec3& VEC3) const { return Vec3(x + VEC3.x, y + VEC3.y, z + VEC3.z); }
	Vec3 Vec3::operator - (const Vec3& VEC3) const { return Vec3(x - VEC3.x, y - VEC3.y, z - VEC3.z); }
	Vec3 Vec3::operator * (const Vec3& VEC3) const { return Vec3(x * VEC3.x, y * VEC3.y, z * VEC3.z); }
	Vec3 Vec3::operator / (const Vec3& VEC3) const { return Vec3(x / VEC3.x, y / VEC3.y, z / VEC3.z); }
	bool Vec3::operator == (const Vec3& VEC3) const { return (x == VEC3.x && y == VEC3.y && z == VEC3.z); }
	bool Vec3::operator != (const Vec3& VEC3) const { return (x != VEC3.x || y != VEC3.y || z != VEC3.z); }
	Vec3 Vec3::operator += (const GLfloat& SCALER) { x += SCALER; y += SCALER; y += SCALER; return *this; }
	Vec3 Vec3::operator -= (const GLfloat& SCALER) { x -= SCALER; y -= SCALER; y -= SCALER; return *this; }
	Vec3 Vec3::operator *= (const GLfloat& SCALER) { x *= SCALER; y *= SCALER; y *= SCALER; return *this; }
	Vec3 Vec3::operator /= (const GLfloat& SCALER) { x /= SCALER; y /= SCALER; y /= SCALER; return *this; }
	Vec3 Vec3::operator + (const GLfloat& SCALER) const { return Vec3(x + SCALER, y + SCALER, z + SCALER); }
	Vec3 Vec3::operator - (const GLfloat& SCALER) const { return Vec3(x - SCALER, y - SCALER, z - SCALER); }
	Vec3 Vec3::operator * (const GLfloat& SCALER) const { return Vec3(x * SCALER, y * SCALER, z * SCALER); }
	Vec3 Vec3::operator / (const GLfloat& SCALER) const { return Vec3(x / SCALER, y / SCALER, z / SCALER); }
	void Vec3::rotate(const GLfloat ANGLE, Axis axis) {
		switch (axis) {
		case X:
			z = z * cos(ANGLE) - y * sin(ANGLE);
			y = z * sin(ANGLE) + y * cos(ANGLE);
			break;
		case Y:
			x = x * cos(ANGLE) - z * sin(ANGLE);
			z = x * sin(ANGLE) + z * cos(ANGLE);
			break;
		case Z:
			x = x * cos(ANGLE) - y * sin(ANGLE);
			y = x * sin(ANGLE) + y * cos(ANGLE);
			break;
		default:
			break;
		}
	}

	Vec2::Vec2(GLfloat x, GLfloat y) : x(x), y(y) {}
	Vec2::Vec2(const glm::vec2& VEC2) : x(VEC2.x), y(VEC2.y) {}
	Vec2::Vec2(const glm::ivec2& VEC2) : x(VEC2.x), y(VEC2.y) {}
	Vec2::operator glm::vec2() const { return glm::vec2(x, y); }
	Vec2 Vec2::operator = (const Vec2& VEC2) { x = VEC2.x; y = VEC2.y; return VEC2; }
	Vec2 Vec2::operator += (const Vec2& VEC2) { x += VEC2.x; y += VEC2.y; return *this; }
	Vec2 Vec2::operator -= (const Vec2& VEC2) { x -= VEC2.x; y -= VEC2.y; return *this; }
	Vec2 Vec2::operator *= (const Vec2& VEC2) { x *= VEC2.x; y *= VEC2.y; return *this; }
	Vec2 Vec2::operator /= (const Vec2& VEC2) { x /= VEC2.x; y /= VEC2.y; return *this; }
	Vec2 Vec2::operator + (const Vec2& VEC2) const { return Vec2(x + VEC2.x, y + VEC2.y); }
	Vec2 Vec2::operator - (const Vec2& VEC2) const { return Vec2(x - VEC2.x, y - VEC2.y); }
	Vec2 Vec2::operator * (const Vec2& VEC2) const { return Vec2(x * VEC2.x, y * VEC2.y); }
	Vec2 Vec2::operator / (const Vec2& VEC2) const { return Vec2(x / VEC2.x, y / VEC2.y); }
	bool Vec2::operator == (const Vec2& VEC2) const { return (x == VEC2.x && y == VEC2.y); }
	bool Vec2::operator != (const Vec2& VEC2) const { return (x != VEC2.x || y != VEC2.y); }
	Vec2 Vec2::operator += (const GLfloat& SCALER) { x += SCALER; y += SCALER; return *this; }
	Vec2 Vec2::operator -= (const GLfloat& SCALER) { x -= SCALER; y -= SCALER; return *this; }
	Vec2 Vec2::operator *= (const GLfloat& SCALER) { x *= SCALER; y *= SCALER; return *this; }
	Vec2 Vec2::operator /= (const GLfloat& SCALER) { x /= SCALER; y /= SCALER; return *this; }
	Vec2 Vec2::operator + (const GLfloat& SCALER) const { return Vec2(x + SCALER, y + SCALER); }
	Vec2 Vec2::operator - (const GLfloat& SCALER) const { return Vec2(x - SCALER, y - SCALER); }
	Vec2 Vec2::operator * (const GLfloat& SCALER) const { return Vec2(x * SCALER, y * SCALER); }
	Vec2 Vec2::operator / (const GLfloat& SCALER) const { return Vec2(x / SCALER, y / SCALER); }
	void Vec2::rotate(const GLfloat ANGLE) {
		x = x * cos(ANGLE) - y * sin(ANGLE);
		y = x * sin(ANGLE) + y * cos(ANGLE);
	}

	Color::Color(GLubyte r, GLubyte g, GLubyte b, GLubyte a) : r(r), g(g), b(b), a(a) {}
	Color Color::operator = (const Color& COLOR) {
		this->r = COLOR.r; this->g = COLOR.g; this->b = COLOR.b; this->a = COLOR.a; return COLOR;
	}
	Color Color::operator += (const Color& COLOR) {
		this->r += COLOR.r; this->g += COLOR.g; this->b += COLOR.b; return *this;
	}
	Color Color::operator -= (const Color& COLOR) {
		this->r -= COLOR.r; this->g -= COLOR.g; this->b -= COLOR.b; return *this;
	}
	Color Color::operator *= (const Color& COLOR) {
		this->r *= COLOR.r; this->g *= COLOR.g; this->b *= COLOR.b; return *this;
	}
	Color Color::operator /= (const Color& COLOR) {
		this->r /= COLOR.r; this->g /= COLOR.g; this->b /= COLOR.b; return *this;
	}
	Color Color::operator + (const Color& COLOR) const {
		return Color(this->r + COLOR.r, this->g + COLOR.g, this->b + COLOR.b, this->a);
	}
	Color Color::operator - (const Color& COLOR) const {
		return Color(this->r - COLOR.r, this->g - COLOR.g, this->b - COLOR.b, this->a);
	}
	Color Color::operator * (const Color& COLOR) const {
		return Color(this->r * COLOR.r, this->g * COLOR.g, this->b * COLOR.b, this->a);
	}
	Color Color::operator / (const Color& COLOR) const {
		return Color(this->r / COLOR.r, this->g / COLOR.g, this->b / COLOR.b, this->a);
	}
	Color Color::operator += (const int& VALUE) {
		this->r += VALUE; this->g += VALUE; this->b += VALUE; return *this;
	}
	Color Color::operator -= (const int& VALUE) {
		this->r -= VALUE; this->g -= VALUE; this->b -= VALUE; return *this;
	}
	Color Color::operator *= (const int& VALUE) {
		this->r *= VALUE; this->g *= VALUE; this->b *= VALUE; return *this;
	}
	Color Color::operator /= (const int& VALUE) {
		this->r /= VALUE; this->g /= VALUE; this->b /= VALUE; return *this;
	}
	Color Color::operator + (const int& VALUE) const {
		return Color(this->r + VALUE, this->g + VALUE, this->b + VALUE, this->a);
	}
	Color Color::operator - (const int& VALUE) const {
		return Color(this->r - VALUE, this->g - VALUE, this->b - VALUE, this->a);
	}
	Color Color::operator * (const int& VALUE) const {
		return Color(this->r * VALUE, this->g * VALUE, this->b * VALUE, this->a);
	}
	Color Color::operator / (const int& VALUE) const {
		return Color(this->r / VALUE, this->g / VALUE, this->b / VALUE, this->a);
	}
	bool Color::operator == (const Color& COLOR) const {
		if (this->r == COLOR.r) {
			if (this->g == COLOR.g) {
				if (this->b == COLOR.b) {
					if (this->a == COLOR.a) {
						return true;
					}
				}
			}
		}
		return false;
	}
	bool Color::operator != (const Color& COLOR) const {
		if (this->r != COLOR.r) { return true; }
		if (this->g != COLOR.g) { return true; }
		if (this->b != COLOR.b) { return true; }
		if (this->a != COLOR.a) { return true; }
		return false;
	}

}