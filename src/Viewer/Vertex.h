#ifndef VERTEX_H
#define VERTEX_H

#include <GL\glew.h>

struct Position {

	GLfloat x{ 0 }, y{ 0 }, z{ 0 };

	Position() = default;
	Position(GLfloat x, GLfloat y, GLfloat z) : x(x), y(y), z(z) {};
};

struct UVCords {

	GLfloat u{ 0 }, v{ 0 };

	UVCords() = default;
	UVCords(GLfloat u, GLfloat v) : u(u), v(v) {};
};

#endif