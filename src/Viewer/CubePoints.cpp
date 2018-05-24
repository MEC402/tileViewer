#include "stdafx.h"
#include <cmath>
#include <time.h>

/* ----------- Layout of current data for each quad face ----------- */
/* -------------------- [ x y z | face | depth ] ------------------- */
/* ----------------------------------------------------------------- */

CubePoints::CubePoints(int maxResDepth) : m_maxResDepth((int)pow(2, maxResDepth) - 1)
{
	// Number of quads per axis
	m_faceDimensions = (int)pow(2, maxResDepth);

	// Total number of quads per face
	m_faceQuads = m_faceDimensions * m_faceDimensions;

	// 6 faces * number of quads per face * amount of data per quad
	m_positions.resize(6 * m_faceQuads * m_datasize);

	// Amount of data per row for any given face
	m_perRow = m_faceDimensions * m_datasize;

	// How far over is our next quad center point
	m_TILESTEP = (1.0f / (float)m_faceDimensions);

	// How far away the edges of a quad are from its center point
	m_TILEWIDTH = m_TILESTEP / 2.0f;

	// How far away from the origin to place quads
	m_faceDistance = m_TILESTEP * ((float)m_faceDimensions / 2.0f);

	// Number of points to draw in OpenGL calls
	m_NumVertices = (GLuint)(6 * m_faceQuads);

	// Generate points for each of the 6 faces
	for (int face = 0; face < 6; face++) {

		// Where in our vector to begin
		int faceBegin = (m_faceQuads * m_datasize) * face;
		// Where in our vector to end
		int faceEnd = (m_faceQuads * m_datasize) * (face + 1);

		// Starting X/Y positions for a given face
		float quadX = -m_TILEWIDTH * m_maxResDepth;
		float quadY = m_TILEWIDTH * m_maxResDepth;

		// Offsets are incremented/decremented appropriately in the nested loop
		float xOffset = 0.0f;
		float yOffset = 0.0f;

		// Point positional data
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		for (int quadPoint = faceBegin, xCoord = 0, yCoord = 0; quadPoint < faceEnd; xCoord++) {

			// Populate our map with vertex buffer offset for this quad
			m_tileMap[face][yCoord][xCoord][0] = quadPoint;
			// Default depth of 0
			m_tileMap[face][yCoord][xCoord][1] = 0;

			// Set xyz position for vertex and plane direction for geometry shader to build quads in
			switch (face) {
			case 0: // Front face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = -m_faceDistance;
				break;
			case 1: // Back face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = m_faceDistance;
				break;
			case 2: // Right face
				x = m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				break;
			case 3: // Left face
				x = -m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				break;
			case 4: // Top face
				x = quadX + xOffset;
				y = m_faceDistance;
				z = quadY + yOffset;
				break;
			case 5: // Bottom face
				x = quadX + xOffset;
				y = -m_faceDistance;
				z = quadY + yOffset;
				break;
			}

			// xyz coordinates for the center point of the quad
			m_positions[quadPoint++] = x;
			m_positions[quadPoint++] = y;
			m_positions[quadPoint++] = z;

			// So we know which texture to use
			m_positions[quadPoint++] = (float)face;

			// Set the depth value of the quad in the vertex buffer
			m_positions[quadPoint++] = 0.0f;

			xOffset += m_TILESTEP;
			if (quadPoint != faceBegin && quadPoint % m_perRow == 0) {
				xOffset = 0.0f;
				yOffset -= m_TILESTEP;
				yCoord++;
				// We're in a for loop so this automatically increments up to 0
				xCoord = -1;
			}
		}
	}
	m_setupOGL();
}

// Get the current depth of a given face
int CubePoints::FaceCurrentDepth(int face)
{
	int faceIndex = face * (m_datasize * m_faceQuads);
	return (int)m_positions[faceIndex + 10];
}

// Set an entire faces quads to the next depth
void CubePoints::FaceNextDepth(int face)
{
	int startIndex = face * (m_datasize * m_faceQuads);
	int currentDepth = (int)m_positions[startIndex + m_datasize - 1];
	for (int i = 0, j = m_datasize - 1; i < m_faceQuads; i++, j += m_datasize) {
		m_positions[(startIndex + j)] = (float)(currentDepth + 1);
	}
	Ready = true;
}

// Easier way to call CubePoints functions with std::thread library
std::thread CubePoints::FaceNextDepthThread(int face)
{
	return std::thread([=] { FaceNextDepth(face); });
}

// Get the current depth of a given quad
int CubePoints::QuadCurrentDepth(int face, int row, int col)
{
	row = (m_faceDimensions - 1) - row;
	return m_tileMap[face][row][col][1];
}

void CubePoints::QuadNextDepth(int face, int row, int col)
{
	// Depending on the face, it's smoother to update rows or columns in reverse when loading in next
	// texture levels.  They'll always get the right texture in the right spot eventually, but there's
	// a strange ~3ish frame delay between the depth update and the correct texture being rendered if we
	// don't do this
	if (face != 5) {
		row = (m_faceDimensions - 1) - row;
	}
	if (face == 1 || face == 3) {
		col = (m_faceDimensions - 1) - col;
	}

	// Get our vertex buffer index offset for the given quad
	int quadToChange = m_tileMap[face][row][col][0];
	// Get our next depth level of said quad
	int nextDepth = m_tileMap[face][row][col][1] + 1;

	// Maaaagic numbers
	if (nextDepth > 3) {
		fprintf(stderr, "Tile group is already at max depth, skipping\n");
		return;
	}

	// Quads per axis / Depth level  -> How many quads in each axis need updating
	int numQuadsToChange = m_faceDimensions / (int)pow(2, nextDepth);

	// Calculate what the new level position of a quad will be
	// e.g. Level 1 might be (0,0) or (1,0) etc
	// This is so we can update groups of quads 
	int depthQuadRow = row / numQuadsToChange;
	int depthQuadCol = col / numQuadsToChange;

	// What row/col we want to start making changes on
	// e.g. If we passed in row 3, col 2 but are going to Depth 1, this would offset us to (0,0)
	// This makes it easier to iteratively update a group of quads
	int startRow = numQuadsToChange * depthQuadRow;
	int startCol = numQuadsToChange * depthQuadCol;


	// This looks horrifying but it's just because of nested arrays
	// m_tilemap[][][]	-> Vertex buffer offset
	// startRow + i		-> Which row the next quad lives on (global coords)
	// startCol + j		-> Which col the next quad lives on (global coords)
	// m_datasize - 1	-> Depth value for a quad is the last index per quad, so however_much_data - 1 to get offset
	for (int i = 0; i < numQuadsToChange; i++) {
		for (int j = 0; j < numQuadsToChange; j++) {
			m_tileMap[face][startRow + i][startCol + j][1] = nextDepth;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 1] = (float)nextDepth;
		}
	}

	// We've updated something, set our Ready flag to true so we can update our VBO
	Ready = true;
}

// Easier way to thread CubePoints:: calls with std::thread library
std::thread CubePoints::QuadNextDepthThread(int face, int row, int col)
{
	return std::thread([=] { QuadNextDepth(face, row, col); });
}

void CubePoints::RebindVAO()
{
	Ready = false;

	//TODO: Need some way to only update changed quad positions instead of repushing whole array
	// Pushing entire array is m_datasize * 6 * quads per face
	// As of 05/24/18 12:02PM that is: 5 * 6 * 64 -> 1920 * sizeof(float) -> 7.5MiB
	glBindVertexArray(m_PositionVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	glBufferSubData(GL_ARRAY_BUFFER, 0, m_positions.size() * sizeof(float), &m_positions.front());
	glBindVertexArray(0);
}

void CubePoints::m_setupOGL()
{
	// Binding this will create a vertex buffer in your GPU
	// https://www.opengl.org/sdk/docs/man/html/glGenBuffers.xhtml
	glGenBuffers(1, &m_PositionVBOID);
	glGenVertexArrays(1, &m_PositionVAOID);
	glBindVertexArray(m_PositionVAOID);

	// "Open" a named buffer object
	// https://www.opengl.org/sdk/docs/man/html/glBindBuffer.xhtml
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);

	// Give data
	// https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), &m_positions.front(), GL_STATIC_DRAW);

	// Bind our xyz
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), 0);

	// Which face the quad belongs to
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(3 * sizeof(float)));

	// Depth level
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(4 * sizeof(float)));

	glBindVertexArray(0);
}