#include "stdafx.h"
#include <time.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

CubePoints::CubePoints(int maxResDepth) : m_maxResDepth(maxResDepth) {
	m_faceDimensions = maxResDepth + 1;
	m_faceQuads = m_faceDimensions * m_faceDimensions;
	m_positions.resize(6 * m_faceQuads * m_datasize);
	m_perRow = m_faceDimensions * m_datasize;

	m_TILESTEP = (1.0f / (float)m_faceDimensions);
	m_TILEWIDTH =  m_TILESTEP / 2.0f;

	// Not sure if this is necessary now?
	m_faceDistance = m_TILESTEP * ((float)m_faceDimensions / 2.0f);
	m_NumVertices = (GLuint)(6 * m_faceQuads);
	srand(time(NULL));

	const float vertexIncrement = 2.0f / m_faceDimensions;

	// Generate points for each of the 6 faces
	for (int face = 0; face < 6; ++face) {
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

			// Pretty much just repeated what Cube.cpp() does when generating triangle quads, except the geometry
			// shader appears to solve the issue of culling triangles for us so mirrored faces don't need flipped coordinates
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
				g_x = 0.00f; //0.10f;
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
			// This will later be replaced with uv maps
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			m_positions[quadPoint++] = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);



			xOffset += m_TILESTEP;
			if (quadPoint != faceBegin && quadPoint % m_perRow == 0) {
				xOffset = 0.0f;
				yOffset -= m_TILESTEP;
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

	
	// Bind our geometry plane
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(3 * sizeof(float)));

	// Bind our rgb
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(6 * sizeof(float)));

	// Bind our TX coords
	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(6 * sizeof(float)));
	//glEnableVertexAttribArray(3);
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, m_datasize * sizeof(float), (void*)(8 * sizeof(float)));

	glBindVertexArray(0);

	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
										   // set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
	unsigned char *data = stbi_load("container.jpg", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		fprintf(stderr, "Failed to load image");
	}
	stbi_image_free(data);

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