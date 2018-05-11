#include "stdafx.h"


CubePoints::CubePoints(int maxResDepth) : m_maxResDepth(maxResDepth) {
	m_faceDimensions = maxResDepth + 1;
	m_faceQuads = m_faceDimensions * m_faceDimensions;
	m_numDataPoints = m_faceQuads * 6 * 6;
	m_positions.resize(m_faceQuads);
	m_uvs.resize(m_faceQuads);
	m_NumVertices = (GLsizei)m_numDataPoints;
	
	
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