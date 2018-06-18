#include "GraphicalMenu.h"
#include "Render.h"

void GraphicalMenu::create(std::vector<PanoInfo> panoList)
{
	shader.CreateProgram(0, "gui.vert", "gui.frag");
	createModelFromQuad(&quad);

	std::vector<ImageData*> thumbnailFiles;
	std::vector<std::string> urls;
	for (unsigned int i = 0; i < panoList.size(); ++i)
	{
		urls.push_back(panoList[i].thumbAddress);
		thumbnailFiles.push_back(new ImageData);
	}

	downloadMultipleFiles(thumbnailFiles.data(), urls.data(), urls.size());
	thumbnailCount = panoList.size();
	thumbnails = new GLuint[panoList.size()];

	for (unsigned int i = 0; i < panoList.size(); ++i)
	{
		int width, height, nrChannels;
		unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)thumbnailFiles[i]->data, thumbnailFiles[i]->dataSize, &width, &height, &nrChannels, 0));

		thumbnails[i] = createTexture(THUMB_TX_SLOT, width, height, GL_RGB, d);

		free(thumbnailFiles[i]->data);
		stbi_image_free(d);
	}

	for (unsigned int i = 0; i < thumbnailFiles.size(); ++i) {
		delete thumbnailFiles[i];
	}
}

void GraphicalMenu::display(glm::quat headsetRotation, glm::mat4x4 viewProjection, 
	float radius, float panoSelection, bool tilt)
{
	float tileSeparation = 0.4f;
	float menuRotation = panoSelection*tileSeparation;
	glDisable(GL_DEPTH_TEST);
	shader.Bind();

	// Limit the tiles drawn so that they don't wrap all they way around the ring
	int peripheralThumbnailCount = 3;
	int minTile = panoSelection - peripheralThumbnailCount;
	if (minTile < 0) minTile = 0;
	int maxTile = panoSelection + peripheralThumbnailCount;
	if (maxTile >= thumbnailCount) maxTile = thumbnailCount-1;

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
		glm::mat4x4 scale = glm::scale(glm::vec3(tileScale, tileScale, tileScale));
		glm::mat4x4 model;
		if (tilt) {
			glm::mat4x4 tiltDown = glm::rotate(glm::radians(-20.0f), glm::vec3(1, 0, 0));
			model = rotation * tiltDown*translation*scale;
		}
		else {
			model = rotation * translation * scale;
		}
		

		shader.SetMatrixUniform("MVP", viewProjection*model);
		shader.BindTexture("image", THUMB_TX_SLOT, thumbnails[i]); // The first 12 textures slots are used for panorama cube faces
		renderModel(quad);
	}
}

// TODO: This overload is only repeated in full to avoid the tiltDown matrix inclusion
// Need to refactor this so we're not duplicating an entire function
void GraphicalMenu::display(glm::quat cameraRotation, glm::mat4x4 viewProjection, float panoSelection)
{
	float radius = 0.65;
	display(cameraRotation, viewProjection, radius, panoSelection, false);
}