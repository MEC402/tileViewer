#include "Cube.h"
#include <GL\glew.h>
#include "Vertex.h"


Cube::Cube(int maxResDepth) : maxResDepth(maxResDepth){

	// Generate vertex array object names
	// https://www.opengl.org/sdk/docs/man/html/glGenVertexArrays.xhtml
	// Binding this will save the following code settings for later use
	// https://www.opengl.org/sdk/docs/man/html/glBindVertexArray.xhtml
	glGenVertexArrays(1, &m_VAOID);
	glBindVertexArray(m_VAOID);

	// Binding this will create a vertex buffer in your GPU
	// https://www.opengl.org/sdk/docs/man/html/glGenBuffers.xhtml
	glGenBuffers(1, &m_PositionVBOID);
	// "Open" a named buffer object
	// https://www.opengl.org/sdk/docs/man/html/glBindBuffer.xhtml
	glBindBuffer(GL_ARRAY_BUFFER, m_PositionVBOID);
	// Give data
	// https://www.opengl.org/sdk/docs/man/html/glBufferData.xhtml
	glBufferData(GL_ARRAY_BUFFER, sizeof(Position) * positions.size(), positions.data(), GL_DYNAMIC_DRAW);
	// Enable generic vertex attribute arrays
	// https://www.opengl.org/sdk/docs/man/html/glEnableVertexAttribArray.xhtml
	glEnableVertexAttribArray(0);
	// https://www.opengl.org/sdk/docs/man/html/glVertexAttribPointer.xhtml
	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	glGenBuffers(1, &m_UVCordsVBOID);
	glBindBuffer(GL_ARRAY_BUFFER, m_UVCordsVBOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(UVCords) * uvs.size(), uvs.data(), GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(1);
	// UV attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// "Close" VBOs
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	// "Close" VAO
	glBindVertexArray(0);
}
