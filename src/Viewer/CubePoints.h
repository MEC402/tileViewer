#pragma once
#include <vector>
#include "Vertex.h"

class CubePoints {

public:

	CubePoints(int maxResDepth);
	~CubePoints() = default;

	void doubleUVs();
	GLuint m_PositionVBOID{ 0 };
	GLuint m_PositionVAOID{ 0 };
	GLsizei m_NumVertices{ 0 };

private:

	int m_maxResDepth{ 0 };
	int m_currentResDepth{ 0 };


	// Magic hardcoded number please do not forget about me
	int m_datasize{ 9 };
	// No really don't forget about it look right here


	int m_faceDimensions{ 0 };
	int m_faceQuads{ 0 };
	int m_perRow{ 0 };
	float m_faceDistance{ 0.0f };

	const float TILEWIDTH = 0.05f;
	const float TILESTEP = 0.1f;

	std::vector<float> m_positions;
	std::vector<UVCords> m_uvs;
	GLuint m_VAOID{ 0 };
	GLuint m_UVCordsVBOID{ 0 };

	void m_setupOGL();
};