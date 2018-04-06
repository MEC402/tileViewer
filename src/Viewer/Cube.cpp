#include "stdafx.h"


Cube::Cube(int maxResDepth) : m_maxResDepth(maxResDepth) {
	m_faceDimentions = maxResDepth + 1;
	m_faceQuads = m_faceDimentions * m_faceDimentions;
	m_numVerticies = m_faceQuads * 6 * 6;
	m_positions.resize(m_numVerticies);
	m_uvs.resize(m_numVerticies);

	// Make an array which maps the structure of a quad in triangles
	// corner{x,y}
	// Triangle 1
	// 0{0,0} 1{1,0}
	// 2{0,1}
	// Triangle 2
	//        4{1,0}
	// 3{0,1} 5{1,1}
	float xMod[]{ 0, 1, 0, 0, 1, 1 };
	float yMod[]{ 0, 0, 1, 1, 0, 1 };

	const float vertexIncrement = 2.0f / m_faceDimentions;
	// For each face
	for (int face = 0; face < 6; ++face) {
		// Get the start and end indecies for this face in 
		int faceBegin = (m_numVerticies / 6) * face;
		int faceEnd = (m_numVerticies / 6) * (face + 1);
		// Set the position of the face's first quad
		float quadX = -1;
		float quadY = -1;
		// For each quad
		for (int quadVertex = faceBegin; quadVertex < faceEnd; quadVertex += 6) {
			// For each corner
			for (int corner = 0; corner < 6; ++corner) {
				int vertex = quadVertex + corner;
				float vertexX = quadX + (vertexIncrement * xMod[corner]);
				float vertexY = quadY + (vertexIncrement * yMod[corner]);
				m_uvs[vertex].u = (vertexX + 1) / 2;
				m_uvs[vertex].v = (vertexY + 1) / 2;
				switch (face) {
				case 0: // Front
					m_positions[vertex].x = vertexX;
					m_positions[vertex].y = vertexY;
					m_positions[vertex].z = 1;
					break;
				case 1: // Back
					m_positions[vertex].x = -vertexX;
					m_positions[vertex].y = -vertexY;
					m_positions[vertex].z = -1;
					break;
				case 2: // Right
					m_positions[vertex].x = 1;
					m_positions[vertex].y = vertexX;
					m_positions[vertex].z = vertexY;
					break;
				case 3: // Left
					m_positions[vertex].x = -1;
					m_positions[vertex].y = -vertexX;
					m_positions[vertex].z = -vertexY;
					break;
				case 4: // Top
					m_positions[vertex].x = vertexX;
					m_positions[vertex].y = 1;
					m_positions[vertex].z = vertexY;
					break;
				case 5: // Bottom
					m_positions[vertex].x = -vertexX;
					m_positions[vertex].y = -1;
					m_positions[vertex].z = -vertexY;
					break;
				}
			}
			if (quadX += vertexIncrement > 1) {
				quadX = -1;
				quadY += vertexIncrement;
			}
		}
	}
	m_setupOGL();
}

void Cube::doubleUVs() {
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

void Cube::m_setupOGL() {

	// Generate vertex array object names
	// https://www.opengl.org/sdk/docs/man/html/glGenVertexArrays.xhtml
	// Binding this will save the following code settings for later use
	// https://www.opengl.org/sdk/docs/man/html/glBindVertexArray.xhtml
	//glGenVertexArrays(1, &m_VAOID);
	//glBindVertexArray(m_VAOID);

	// Binding this will create a vertex buffer in your GPU
	// https://www.opengl.org/sdk/docs/man/html/glGenBuffers.xhtml
	glGenBuffers(1, &m_PositionVBOID);
	// "Open" a named buffer object
	// https://www.opengl.org/sdk/docs/man/html/glBindBuffer.xhtml
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	// Give data
	// https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml
	/*GLfloat* tempArray = new GLfloat[m_positions.size() * 3];
	for (int i = 0; i < m_positions.size(); i++) {
		tempArray[3*i] = m_positions.data()[i].x;
		tempArray[3*i + 1] = m_positions.data()[i].y;
		tempArray[3*i + 2] = m_positions.data()[i].z;
	}
*/
	glBufferData(GL_ARRAY_BUFFER, sizeof(Position) * m_positions.size(), m_positions.data(), GL_STATIC_DRAW);
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