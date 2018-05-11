#include "stdafx.h"
#include <time.h>

CubePoints::CubePoints(int maxResDepth) : m_maxResDepth(maxResDepth) {
	m_faceDimensions = maxResDepth + 1;
	m_faceQuads = m_faceDimensions * m_faceDimensions;
	m_positions.resize(6 * m_faceQuads * m_datasize);
	m_perRow = m_faceDimensions * m_datasize;
	m_faceDistance = TILESTEP * ((float)m_faceDimensions / 2.0f);
	m_NumVertices = (GLuint)(6 * m_faceQuads);
	srand(time(NULL));


	// NOTE: This is set to only generate 4 faces right now, for debugging purposes
	// ######################### pay attention to me so we don't forget about this later ####################
	for (int face = 0; face < 4; ++face) {
		int faceBegin = (m_faceQuads * m_datasize) * face;
		int faceEnd = (m_faceQuads * m_datasize) * (face + 1);

		float quadX = -TILEWIDTH;
		float quadY = TILEWIDTH;
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

			// Pretty much just repeated what Cube.cpp() does when generating triangle quads, except the geometry
			// shader appears to solve the issue of culling triangles for us so mirrored faces don't need flipped coordinates
			switch (face) {
			case 0: // Front face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = -m_faceDistance;
				g_x = TILEWIDTH;
				g_y = TILEWIDTH;
				g_z = 0.00f;
				break;
			case 1: // Back face
				x = quadX + xOffset;
				y = quadY + yOffset;
				z = m_faceDistance;
				g_x = TILEWIDTH;
				g_y = TILEWIDTH;
				g_z = 0.00f;
				break;
			case 2: // Right face
				x = m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f; //0.10f;
				g_y = TILEWIDTH;
				g_z = TILEWIDTH;
				break;
			case 3: // Left face
				x = -m_faceDistance;
				y = quadY + yOffset;
				z = quadX + xOffset;
				g_x = 0.00f;
				g_y = 0.05f;
				g_z = 0.05f;
				break;
			case 4: // Top face
				x = quadX + xOffset;
				y = m_faceDistance;
				z = quadY + yOffset;
				g_x = TILEWIDTH;
				g_y = 0.00f;
				g_z = TILEWIDTH;
				break;
			case 5:
				x = quadX + xOffset;
				y = -m_faceDistance;
				z = quadY + yOffset;
				g_x = TILEWIDTH;
				g_y = 0.00f;
				g_z = TILEWIDTH;
				break;
			}

			// xyz coordinates for the center point of the quad
			m_positions[quadPoint++] = x;
			m_positions[quadPoint++] = y;
			m_positions[quadPoint++] = z;

			// Random colors so we can see if we're generating the right number of quads per face
			// This will later be replaced with uv maps
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			// What planes we want to create the quad on in our geometry shader
			m_positions[quadPoint++] = g_x;
			m_positions[quadPoint++] = g_y;
			m_positions[quadPoint++] = g_z;

			xOffset += TILESTEP;
			if (quadPoint != 0 && quadPoint % m_perRow == 0) {
				xOffset = 0.0f;
				yOffset -= TILESTEP;
			}
		}
	}
		
	m_setupOGL();
}

void CubePoints::doubleUVs() {
	if (m_currentResDepth < m_maxResDepth) {
		for (auto&& uv : m_uvs) {
			uv.u *= 2;
			uv.v *= 2;
		}
		++m_currentResDepth;
		glBindBuffer(GL_ARRAY_BUFFER, m_UVCordsVBOID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(UVCords) * m_uvs.size(), m_uvs.data(), GL_DYNAMIC_DRAW);
	}
}

void CubePoints::m_setupOGL() {

	// Generate vertex array object names
	// https://www.opengl.org/sdk/docs/man/html/glGenVertexArrays.xhtml
	// Binding this will save the following code settings for later use
	// https://www.opengl.org/sdk/docs/man/html/glBindVertexArray.xhtml
	//glGenVertexArrays(1, &m_VAOID);
	//glBindVertexArray(m_VAOID);

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

	// Bind our rgb
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(3 * sizeof(float)));

	// Bind our geometry plane
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(6 * sizeof(float)));
	glBindVertexArray(0);

	// Enable generic vertex attribute arrays
	// https://www.opengl.org/sdk/docs/man/html/glEnableVertexAttribArray.xhtml
	//glEnableVertexAttribArray(0);
	// https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml
	// Position attribute
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	//glGenBuffers(1, &m_UVCordsVBOID);
	//glBindBuffer(GL_ARRAY_BUFFER, m_UVCordsVBOID);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(UVCords) * m_uvs.size(), m_uvs.data(), GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(1);
	//// UV attribute
	//glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	//// "Close" VBOs
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//// "Close" VAO
	//glBindVertexArray(0);
}