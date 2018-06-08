#include "CubePoints.h"
#include <cmath>
#include <time.h>

#include <mutex>

/* ----------- Layout of current data for each quad face ----------- */
/* -------------------- [ x y z | face | depth ] ------------------- */
/* ----------------------------------------------------------------- */

std::mutex m_;

CubePoints::CubePoints(int maxResDepth, int m_eye) : 
	m_maxResDepth((int)pow(2, maxResDepth) - 1),
	m_eye(m_eye)
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

			//xyz coordinates for the center point of the quad
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

bool CubePoints::Ready()
{
	return !m_VBOupdates.empty();
}

void CubePoints::QuadSetDepth(int face, int row, int col, int depth)
{
	std::lock_guard<std::mutex> lock(m_);

	if (face != 5)
		row = (m_faceDimensions - 1) - row;
	if (face == 1 || face == 3)
		col = (m_faceDimensions - 1) - col;

	// Hopefully prevents loading lower-res tiles over high-res tiles?
	if (m_tileMap[face][row][col][1] > depth) {
		return;
	}
		
	// Bunch of offset math so we can update groups of quads if we're not at the lowest depth
	int quadToChange = m_tileMap[face][row][col][0];
	int numQuadsToChange = m_faceDimensions / (int)pow(2, depth);
	int depthQuadRow = row / numQuadsToChange;
	int depthQuadCol = col / numQuadsToChange;
	int startRow = numQuadsToChange * depthQuadRow;
	int startCol = numQuadsToChange * depthQuadCol;

	for (int i = 0; i < numQuadsToChange; i++) {
		for (int j = 0; j < numQuadsToChange; j++) {
			m_tileMap[face][startRow + i][startCol + j][1] = depth;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 1] = (float)depth;
		}
	}
	m_VBOupdates.emplace_back(std::tuple<int, int>(
		m_tileMap[face][startRow][startCol][0],
		m_tileMap[face][startRow + numQuadsToChange - 1][startCol + numQuadsToChange - 1][0] + m_datasize
		));
}

void CubePoints::ResetDepth()
{
	std::lock_guard<std::mutex> lock(m_);
	for (unsigned int i = m_datasize - 1; i < m_positions.size(); i += m_datasize) {
		m_positions[i] = 0.0f;
	}

	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < m_faceDimensions; j++) {
			for (int k = 0; k < m_faceDimensions; k++) {
				m_tileMap[i][j][k][1] = 0;
			}
		}
	}

	m_VBOupdates.emplace_back(std::tuple<int, int>(0, m_positions.size()));
}

void CubePoints::RebindVAO()
{
	std::lock_guard<std::mutex> lock(m_);

	if (m_VBOupdates.empty())
		return;

	std::tuple<int, int> Range = m_VBOupdates.front();
	int front = std::get<0>(Range);
	int back = std::get<1>(Range);
	m_VBOupdates.pop_front();

#ifdef DEBUG
	GLenum error;
#endif
	glBindVertexArray(m_PositionVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	if (m_buffer)
		std::memcpy(&m_buffer[int(front)], &m_positions[front], (back - front) * sizeof(float));
#ifdef DEBUG
	else
		fprintf(stderr, "MapBuffer pointer is NULL\n");
#endif

	glFlushMappedBufferRange(GL_ARRAY_BUFFER, front * sizeof(float), (back - front) * sizeof(float));
#ifdef DEBUG
	if ((error = glGetError()) != GL_NO_ERROR) {
		fprintf(stderr, "Error flushing to MapBuffer: %s\n", gluErrorString(error));
	}
#endif
	glBindBuffer(GL_ARRAY_BUFFER, 0);
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
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), &m_positions.front(), GL_STATIC_DRAW);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), &m_positions.front(),
		GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

	// Bind our xyz
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), 0);

	// Which face the quad belongs to
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(3 * sizeof(float)));

	// Depth level
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(4 * sizeof(float)));

	m_buffer = (float*)glMapBufferRange(GL_ARRAY_BUFFER, 0, m_positions.size() * sizeof(float),
		GL_MAP_WRITE_BIT | GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_PERSISTENT_BIT);

	glBindVertexArray(m_eye);
}