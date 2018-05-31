#ifndef CUBEPOINTS_H
#define CUBEPOINTS_H

#include <thread>
#include <vector>
#include "Vertex.h"


class CubePoints {

public:

	CubePoints(int maxResDepth, int eye);
	~CubePoints() = default;

	void RebindVAO();
	void QuadSetDepth(int face, int row, int col, int depth);
	void ResetDepth();

	GLuint m_PositionVBOID{ 0 };
	GLuint m_PositionVAOID{ 0 };
	GLsizei m_NumVertices{ 0 };

	// Magic hardcoded number please do not forget about me
	// xyz face depth (3 + 1 + 1)
	int m_datasize{ 5 };

	// How far away vertices for a quad center to place (used in geometry shader)
	float m_TILEWIDTH{ 0.0f };
	
	bool Ready{ false };

private:

	int m_maxResDepth{ 0 };
	int m_currentResDepth{ 0 };

	// Quads per axis
	int m_faceDimensions{ 0 };
	// Quads per face
	int m_faceQuads{ 0 };
	// Data points per row
	int m_perRow{ 0 };
	// How far from the origin to place points
	float m_faceDistance{ 0.0f };
	// How far over the next quad is
	float m_TILESTEP{ 0.0f };

	// 6 faces
	// 8 quads X direction
	// 8 quads Y direction
	// Index of tile
	// Depth of tile
	int m_tileMap[6][8][8][2]{ { { { 0 } } } };
	std::vector<float> m_positions;

	int eye;

	void m_setupOGL();
	
};

#endif