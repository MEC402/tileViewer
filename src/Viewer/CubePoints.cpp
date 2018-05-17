#include "stdafx.h"
#include <cmath>
#include <time.h>


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

		for (int quadPoint = faceBegin; quadPoint < faceEnd;) {

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

			m_positions[quadPoint++] = 3.0f;

			xOffset += m_TILESTEP;
			if (quadPoint != faceBegin && quadPoint % m_perRow == 0) {
				xOffset = 0.0f;
				yOffset -= m_TILESTEP;
			}
		}
	}
	m_setupOGL();
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