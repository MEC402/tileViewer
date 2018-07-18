#ifndef CUBEPOINTS_H
#define CUBEPOINTS_H

#include <deque>
#include <thread>
#include <vector>
#include <GL\glew.h>
#include "Shared.h"


class CubePoints {

public:

	CubePoints(int maxResDepth, int m_eye);
	~CubePoints() = default;

	void BindVAO(void);
	void RebindVAO(void);
	void QuadSetDepth(int face, int row, int col, int depth);
	void ResetDepth(void);


	GLuint m_PositionVBOID{ 0 };
	GLuint m_PositionVAOID{ 0 };
	GLsizei m_NumVertices{ 0 };

	// Magic hardcoded number please do not forget about me
	// xyz face depth (3 + 1 + 1)
	int m_datasize{ 5 };

	// How far away vertices from a quad center to place (used in geometry shader)
	float m_TILEWIDTH{ 0.0f };
	
private:
	void m_setupOGL(void);
	
	float* m_buffer;

	int m_eye;
	int m_maxResDepth{ 0 };
	int m_faceDimensions{ 0 };
	int m_faceQuads{ 0 };
	int m_perRow{ 0 };
	float m_faceDistance{ 0.0f };
	float m_TILESTEP{ 0.0f };

	std::vector<float> m_positions;

	// 6 faces
	// 8 quads X direction
	// 8 quads Y direction
	// Index of tile
	// Depth of tile
	int m_tileMap[6][8][8][2]{ { { { 0 } } } };

	std::deque<std::tuple<int,int>> m_VBOupdates;	
};

#endif