#pragma once
#include <vector>
#include "Vertex.h"

class Cube {

public:

	Cube(int maxResDepth);
	~Cube() = default;

private:

	int maxResDepth{ 0 };
	int currentResDepth{ 0 };

	std::vector<Position> positions;
	std::vector<UVCords> uvs;
	GLuint m_VAOID{ 0 };
	GLuint m_PositionVBOID{ 0 };
	GLuint m_UVCordsVBOID{ 0 };
};

