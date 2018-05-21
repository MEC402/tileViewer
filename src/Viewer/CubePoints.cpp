#include "stdafx.h"
#include <cmath>
#include <time.h>

/* ----------- Layout of current data for each quad face ----------- */
/* --------- [ x y z | g_x g_y g_z | r g b | face | depth ] -------- */
/* ----------------------------------------------------------------- */

CubePoints::CubePoints(int maxResDepth) : m_maxResDepth(pow(2,maxResDepth) - 1)
{
	m_faceDimensions = pow(2, maxResDepth);
	m_faceQuads = m_faceDimensions * m_faceDimensions;
	m_positions.resize(6 * m_faceQuads * m_datasize);
	m_perRow = m_faceDimensions * m_datasize;

	m_TILESTEP = (1.0f / (float)m_faceDimensions);
	m_TILEWIDTH =  m_TILESTEP / 2.0f;

	m_faceDistance = m_TILESTEP * ((float)m_faceDimensions / 2.0f);
	m_NumVertices = (GLuint)(6 * m_faceQuads);
	srand(time(NULL));

	// Generate points for each of the 6 faces
	for (int face = 0; face < 6; face++) {
		// Where in our vector to begin
		int faceBegin = (m_faceQuads * m_datasize) * face;
		// Where in our vector to end
		int faceEnd = (m_faceQuads * m_datasize) * (face + 1);

		float quadX = -m_TILEWIDTH * m_maxResDepth;
		float quadY = m_TILEWIDTH * m_maxResDepth;
		//float quadX = -1;
		//float quadY = -1;
		float xOffset = 0.0f;
		float yOffset = 0.0f;

		// Point positional data
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		// Geometry "build quad on these points" data
		float g_x = 0.0f;
		float g_y = 0.0f;
		float g_z = 0.0f;

		for (int quadPoint = faceBegin, xCoord = 0, yCoord = 0; quadPoint < faceEnd; xCoord++) {

			m_tileMap[face][yCoord][xCoord][0] = quadPoint;
			m_tileMap[face][yCoord][xCoord][1] = 0;

			// Set xyz position for vertex and plane direction for geometry shader to build quads in
			switch (face) {
			case 0: // Front face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = -m_faceDistance;
				g_x = m_TILEWIDTH;
				g_y = m_TILEWIDTH;
				g_z = 0.00f;
				break;
			case 1: // Back face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = m_faceDistance;
				g_x = m_TILEWIDTH;
				g_y = m_TILEWIDTH;
				g_z = 0.00f;
				break;
			case 2: // Right face
				x = m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f;
				g_y = m_TILEWIDTH;
				g_z = m_TILEWIDTH;
				break;
			case 3: // Left face
				x = -m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f;
				g_y = m_TILEWIDTH;
				g_z = m_TILEWIDTH;
				break;
			case 4: // Top face
				x = quadX + xOffset;
				y = m_faceDistance;
				z = quadY + yOffset;
				g_x = m_TILEWIDTH;
				g_y = 0.00f;
				g_z = m_TILEWIDTH;
				break;
			case 5:
				x = quadX + xOffset;
				y = -m_faceDistance;
				z = quadY + yOffset;
				g_x = m_TILEWIDTH;
				g_y = 0.00f;
				g_z = m_TILEWIDTH;
				break;
			}

			// xyz coordinates for the center point of the quad
			m_positions[quadPoint++] = x;
			m_positions[quadPoint++] = y;
			m_positions[quadPoint++] = z;

			// What planes we want to create the quad on in our geometry shader
			m_positions[quadPoint++] = g_x;
			m_positions[quadPoint++] = g_y;
			m_positions[quadPoint++] = g_z;

			// Random colors so we can see if we're generating the right number of quads per face
			// This is unnecessary and can be removed from the final product, but it's useful for debugging
			//m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			//m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			//m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			m_positions[quadPoint++] = 1.0f;
			m_positions[quadPoint++] = 1.0f;
			m_positions[quadPoint++] = 1.0f;



			// So we know which texture to use
			m_positions[quadPoint++] = (float)face;

			m_positions[quadPoint++] = 0.0f;

			xOffset += m_TILESTEP;
			if (quadPoint != faceBegin && quadPoint % m_perRow == 0) {
				xOffset = 0.0f;
				yOffset -= m_TILESTEP;
				yCoord++;
				xCoord = -1;
			}
		}
	}
	m_setupOGL();
}

int CubePoints::FaceCurrentDepth(int face)
{
	int faceIndex = face * (m_datasize * m_faceQuads);
	return m_positions[faceIndex + 10];
}

void CubePoints::FaceNextDepth(int face)
{
	int startIndex = face * (m_datasize * m_faceQuads);
	int currentDepth = m_positions[startIndex + m_datasize - 1];
	for (int i = 0, j = m_datasize-1; i < m_faceQuads; i++, j += m_datasize) {
		m_positions[(startIndex + j)] = (float)(currentDepth + 1);
	}


	//TODO: This might be an incredibly expensive way to update our VBO/VAO
	// Look into glBufferSubData() and see if we can't use that instead
	glBindVertexArray(m_PositionVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), &m_positions.front(), GL_STATIC_DRAW);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(10 * sizeof(float)));
	
	glBindVertexArray(0);
}

int CubePoints::QuadCurrentDepth(int face, int row, int col)
{
	row = (m_faceDimensions - 1) - row;
	return m_tileMap[face][row][col][1];
	//int faceIndex = face * (m_datasize * m_faceQuads);
	//int quadToChange = faceIndex + (m_datasize * row * m_faceDimensions) + (m_datasize * col);
	//return m_positions[quadToChange + 10];
}

void CubePoints::QuadNextDepth(int face, int row, int col)
{
	// Rows look up ST coordinates in reverse, so we need to update depth levels in reverse
	row = (m_faceDimensions - 1) - row;
	
	// Get the start index of a given face
	//int faceIndex = face * (m_datasize * m_faceQuads);
	//
	//// Find the quad index we want
	//int quadToChange = faceIndex + (m_datasize * row * m_faceDimensions) + (m_datasize * col);
	//
	//// Get the next depth of the quad
	//int nextDepth = m_positions[quadToChange + 10] + 1;

	int quadToChange = m_tileMap[face][row][col][0];
	int nextDepth = m_tileMap[face][row][col][1] + 1;
	fprintf(stderr, "Attempting to send tile %d/%d to depth level %d\n", row, col, nextDepth);
	// Maaaagic numbers
	if (nextDepth > 3) {
		fprintf(stderr, "Tile group is already at max depth, skipping\n");
		return;
	}

	// Number of quads per axis / Depth level tells us how many quads in each axis need updating
	int numQuadsToChange = m_faceDimensions / (int)pow(2, nextDepth);

	// Calculate what the new level position of a quad will be (eg: Level 1 might be 0,0 or 1,0 or 1,1 etc)
	int depthQuadRow = floor(row % (int)pow(2, nextDepth));

	//NOT CALCULATING CORRECTLY
	//int depthQuadCol = floor(col % (int)pow(2, nextDepth));
	int depthQuadCol = col / numQuadsToChange;

	// What is our offset between each row?
	// m_datasize		-> Size of each quad
	// m_faceDimensions	-> Number of quads per axis (row or column)
	int dataPerRow = m_datasize * m_faceDimensions;

	// What row we want to start making changes on
	// numQuadsToChange * depthQuadRow	-> Which row we want
	// * dataPerRow						-> Get the index of the first quad on that row
	//int startRow = numQuadsToChange * depthQuadRow * dataPerRow;
	int startRow = numQuadsToChange * depthQuadRow;


	// What column we want to start making changes on
	// numQuadsToChange * depthQuadCol	-> How many quads to step over
	// * m_datasize						-> Size of each quad to step over
	// MIGHT BE CALCULATING CORRECTLY
	//int startCol = numQuadsToChange * depthQuadCol * m_datasize;
	//int startCol = floor((float)col / (float)numQuadsToChange) * m_datasize;
	int startCol = numQuadsToChange * depthQuadCol;
	int totalOffset = startRow + startCol;

	// totalOffset		-> Our starting row/column index
	// dataPerRow * i	-> Step up a row on each i iteration
	// k				-> Step over one quad at a time
	float r = 0.0f, g = 0.0f, b = 0.0f;
	switch (nextDepth) {
	case 1:
		r = 1.0f;
		break;
	case 2:
		g = 1.0f;
		break;
	case 3:
		b = 1.0f;
		break;
	}

	//int quadX = startRow / m_faceQuads;
	//int quadY = startCol / m_faceDimensions;


	for (int i = 0; i < numQuadsToChange; i++) {
		for (int j = 0, k = m_datasize-1; j < numQuadsToChange; j++, k += m_datasize) {
			//fprintf(stderr, "Setting quad row/col %d/%d to depth level %d\n", quadX + j, quadY + i, nextDepth);
			m_tileMap[face][startRow + i][startCol + j][1] = nextDepth;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 1] = nextDepth;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 3] = b;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 4] = g;
			m_positions[m_tileMap[face][startRow + i][startCol + j][0] + m_datasize - 5] = r;
			//m_positions[totalOffset + (dataPerRow * i) + k] = nextDepth;
			//m_positions[totalOffset + (dataPerRow * i) + k - 2] = b;
			//m_positions[totalOffset + (dataPerRow * i) + k - 3] = g;
			//m_positions[totalOffset + (dataPerRow * i) + k - 4] = r;
		}
	}

	//TODO: This might be an incredibly expensive way to update our VBO/VAO
	// At 11 data points (3 points of color for debugging) we're sending 16MB over the bus if this is a full push
	// At 8 data points (xyz/geometry/face/depth) we're still sending 12MB over the bus
	// Look into glBufferSubData() and see if we can't use that instead
	glBindVertexArray(m_PositionVAOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * m_positions.size(), &m_positions.front(), GL_STATIC_DRAW);

	// Bind our rgb FOR DEBUGGAN
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(6 * sizeof(float)));

	// Bind our depth level
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(10 * sizeof(float)));

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

	// Bind our geometry plane
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(3 * sizeof(float)));

	// Bind our rgb
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(6 * sizeof(float)));

	// Which face the quad belongs to
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(9 * sizeof(float)));

	// Depth level
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(10 * sizeof(float)));

	glBindVertexArray(0);
}