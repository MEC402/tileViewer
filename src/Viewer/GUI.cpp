#include "GUI.h"

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

void renderModel(Model model, Shader& shader, GLuint tex, glm::mat4x4 mvp)
{
	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	// Get a handle for our "MVP" uniform
	shader.Bind();
	shader.SetMatrixUniform("MVP", mvp);
	shader.BindTexture("image", 13, tex); // The first 12 textures slots are used for panorama cube faces

	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexPositionbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.vertexUVBuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Darw
	glDrawArrays(GL_TRIANGLES, 0, model.indices);
	PRINT_GL_ERRORS;
}


void createGUI(GraphicalInterface* gui, std::vector<PanoInfo> panoList)
{
	gui->shader.CreateProgram(0, "gui.vert", "gui.frag");
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

void displayGUI(GraphicalInterface gui, glm::quat headsetRotation, glm::mat4x4 viewProjection, float radius, float panoSelection)
{
	float tileSeparation = 0.4f;
	float menuRotation = panoSelection*tileSeparation;
	glDisable(GL_DEPTH_TEST);

	// Limit the tiles drawn so that they don't wrap all they way around the ring
	int peripheralThumbnailCount = 3;
	int minTile = panoSelection - peripheralThumbnailCount;
	if (minTile < 0) minTile = 0;
	int maxTile = panoSelection + peripheralThumbnailCount;
	if (maxTile >= gui.thumbnailCount) maxTile = gui.thumbnailCount-1;

	for (unsigned int i = minTile; i <= maxTile; ++i)
	{

		float tileScale = 0.05f;
		if (abs(i - panoSelection) < 1.0f) {
			tileScale += (1 - abs(i - panoSelection)) * 0.04f;
		}
		glm::vec3 tilePosition(0, 0, -radius);
		glm::mat4x4 translation = glm::translate(tilePosition);
		glm::vec3 verticalAxis(0, 1, 0);
		// Get camera yaw
		glm::quat q = headsetRotation;
		float cameraYaw = -atan2(2.0*(q.x*q.z - q.w*q.y), q.w*q.w - q.x*q.x - q.y*q.y + q.z*q.z);
		glm::mat4x4 rotation = glm::rotate(menuRotation - (i*tileSeparation) + cameraYaw, verticalAxis);
		glm::mat4x4 tiltDown = glm::rotate(glm::radians(-20.0f), glm::vec3(1, 0, 0));
		glm::mat4x4 scale = glm::scale(glm::vec3(tileScale, tileScale, tileScale));
		glm::mat4x4 model = rotation*tiltDown*translation*scale;

		renderModel(gui.quad, gui.shader, gui.thumbnails[i], viewProjection*model);
	}
}