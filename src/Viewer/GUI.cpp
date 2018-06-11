#include "GUI.h"
#include "GLhandles.h"

void createModelFromQuad(Model* model)
{
	const float positions[] = {
		// Triangle 1
		-1, -1, 0,
		1, -1, 0,
		1, 1, 0,
		// Triangle 2
		1, 1, 0,
		-1, 1, 0,
		-1, -1, 0
	};

	const float uvs[] = {
		// Triangle 1
		0, 0,
		1, 0,
		1, 1,
		// Triangle 2
		1, 1,
		0, 1,
		0, 0
	};

	glGenBuffers(1, &model->vertexPositionbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexPositionbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);

	glGenBuffers(1, &model->vertexUVBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, model->vertexUVBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uvs), uvs, GL_STATIC_DRAW);

	model->indices = 2 * 3;
}

void renderModel(Model model, GLuint tex, OVR::Matrix4f mvp)
{
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	// Get a handle for our "MVP" uniform
	glUseProgram(guiProgram);
	GLuint MatrixID = glGetUniformLocation(guiProgram, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_TRUE, (float*)&mvp);
	
	GLint boundTexture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTexture);

	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);
	GLuint sampler = glGetUniformLocation(guiProgram, "image");
	glUniform1i(sampler, 0);
	//
	//// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexPositionbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//
	//// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);
	//
	//// Darw
	//glDrawArrays(GL_TRIANGLES, 0, model.indices);

	//glActiveTexture(activeTexture);
	glBindTexture(GL_TEXTURE_2D, boundTexture);
	glUseProgram(program);
}


void createGUI(GraphicalInterface* gui, std::vector<PanoInfo> panoList)
{
	createModelFromQuad(&gui->quad);

	std::vector<ImageData*> thumbnails;
	std::vector<std::string> urls;
	for (unsigned int i = 0; i < panoList.size(); ++i)
	{
		urls.push_back(panoList[i].thumbAddress);
		thumbnails.push_back(new ImageData);
	}

	downloadMultipleFiles(thumbnails.data(), urls.data(), urls.size());
	gui->thumbnailCount = panoList.size();
	gui->thumbnails = new GLuint[panoList.size()];

	for (unsigned int i = 0; i < panoList.size(); ++i)
	{
		int width, height, nrChannels;
		unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)thumbnails[i]->data, thumbnails[i]->dataSize, &width, &height, &nrChannels, 0));

		glGenTextures(1, &gui->thumbnails[i]);
		glBindTexture(GL_TEXTURE_2D, gui->thumbnails[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, d);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		free(thumbnails[i]->data);
		stbi_image_free(d);
	}

	for (unsigned int i = 0; i < thumbnails.size(); ++i) {
		delete thumbnails[i];
	}
}

void displayGUI(GraphicalInterface gui, OVR::Matrix4f viewProjection, float panoSelection)
{
	renderModel(gui.quad, gui.thumbnails[0], viewProjection);
}