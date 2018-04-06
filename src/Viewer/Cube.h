#pragma once
#include <vector>
#include "Vertex.h"

class Cube {

public:

	Cube(int maxResDepth);
	~Cube() = default;

	void doubleUVs();
	GLuint m_PositionVBOID{ 0 };

private:

	int m_maxResDepth{ 0 };
	int m_currentResDepth{ 0 };
	int m_faceDimentions{ 0 };
	int m_faceQuads{ 0 };
	int m_numVerticies{ 0 };

	std::vector<Position> m_positions;
	std::vector<UVCords> m_uvs;
	GLuint m_VAOID{ 0 };
	GLuint m_UVCordsVBOID{ 0 };

	void m_setupOGL();
};
