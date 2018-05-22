#pragma once
#include <thread>
#include <vector>
#include "Vertex.h"


class CubePoints {

public:

	CubePoints(int maxResDepth);
	~CubePoints() = default;
	int FaceCurrentDepth(int face);
	void FaceNextDepth(int face);
	std::thread FaceNextDepthThread(int face);

	int QuadCurrentDepth(int face, int row, int col);
	void QuadNextDepth(int face, int row, int col);
	std::thread QuadNextDepthThread(int face, int row, int col);

	GLuint m_PositionVBOID{ 0 };
	GLuint m_PositionVAOID{ 0 };
	GLsizei m_NumVertices{ 0 };

	void RebindVAO();

	// Magic hardcoded number please do not forget about me
	int m_datasize{ 11 };
	// No really don't forget about it look right here
	bool Ready{ false };

private:

	int m_maxResDepth{ 0 };
	int m_currentResDepth{ 0 };
	int m_faceDimensions{ 0 };
	int m_faceQuads{ 0 };
	int m_perRow{ 0 };
	float m_faceDistance{ 0.0f };
	float m_TILEWIDTH{ 0.0f };
	float m_TILESTEP{ 0.0f };

	// 6 faces
	// 8 quads X direction
	// 8 quads Y direction
	// Index of tile
	// Depth of tile
	int m_tileMap[6][8][8][2]{ { { { 0 } } } };
	//std::vector< std::vector <std::vector <std::vector<int>>>> m_tileMap;

	std::vector<float> m_positions;
	std::vector<UVCords> m_uvs;
	GLuint m_VAOID{ 0 };
	GLuint m_UVCordsVBOID{ 0 };

	void m_setupOGL();
	
};
