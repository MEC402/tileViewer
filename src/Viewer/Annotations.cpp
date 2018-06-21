#include "Annotations.h"
#include "InternetDownload.h"
#include "stb_image.h"
#include "Shared.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

Annotations::Annotations()
{
	
}

void Annotations::Create()
{
	Render_CreateQuadModel(&quad);
	shader.CreateProgram(0, "gui.vert", "gui.frag");
}

void Annotations::Load(std::string annotationsJSONAddress)
{
	// Destroy any existing annotations
	for (unsigned int i = 0; i < annotations.size(); ++i) {
		Render_DestroyTexture(&annotations[i].texture);
	}
	annotations.clear();

	// Download JSON file
	if (annotationsJSONAddress.empty()) return;
	ImageData jsonFile;
	downloadFile(&jsonFile, annotationsJSONAddress);
	if (!jsonFile.data) return;

	// Base URL is the substring before the last backslash or forward slash
	size_t lastSlashPosition = annotationsJSONAddress.find_last_of("/\\");
	if (lastSlashPosition == annotationsJSONAddress.npos) {
		lastSlashPosition = 0;
	}
	std::string baseURL = annotationsJSONAddress.substr(0, lastSlashPosition);

	// Read JSON file
	std::string fileAsString(jsonFile.data, jsonFile.data + jsonFile.dataSize);
	annotations = parseAnnotationJSON(fileAsString, baseURL);

	// Download image files
	std::vector<std::string> urls;
	for (unsigned int i = 0; i < annotations.size(); ++i) {
		urls.push_back(annotations[i].filePath);
	}
	std::vector<ImageData> files = downloadMultipleFiles(urls.data(), urls.size());

	// Create a texture for each image
	for (unsigned int i = 0; i< files.size(); ++i)
	{
		if (files[i].data) {
			int width, height, nrChannels;
			unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)files[i].data, files[i].dataSize, &width, &height, &nrChannels, 0));

			GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
			Render_CreateTexture(&annotations[i].texture, THUMB_TX_SLOT, width, height, format, d);

			free(files[i].data);
			stbi_image_free(d);
		}
	}
}

std::vector<Annotations::AnnotationData> Annotations::parseAnnotationJSON(std::string jsonText, std::string baseURL)
{
	std::vector<AnnotationData> annotationList;
	rapidjson::Document jsonDocument;
	jsonDocument.Parse(jsonText.c_str());
	if (!jsonDocument.HasParseError() && jsonDocument.HasMember("annotations"))
	{
		rapidjson::Value& annotationsArray = jsonDocument["annotations"];
		// Iterate through the array of panos
		for (unsigned int i = 0; i < annotationsArray.Size(); ++i)
		{
			AnnotationData a;
			if (annotationsArray[i].HasMember("file")) {
				a.filePath = baseURL + '/' + annotationsArray[i]["file"].GetString();
			}
			if (annotationsArray[i].HasMember("yaw")) {
				a.yaw = annotationsArray[i]["yaw"].GetFloat();
			}
			if (annotationsArray[i].HasMember("pitch")) {
				a.pitch = annotationsArray[i]["pitch"].GetFloat();
			}
			if (annotationsArray[i].HasMember("distance")) {
				a.distance = annotationsArray[i]["distance"].GetFloat();
			}
			if (annotationsArray[i].HasMember("height")) {
				a.height = annotationsArray[i]["height"].GetFloat();
			}
			annotationList.push_back(a);
		}
	}
	else
	{
		printf("Could not read the annotation JSON file; %s", rapidjson::GetParseError_En(jsonDocument.GetParseError()));
	}
	return annotationList;
}

void Annotations::renderAnnotation(AnnotationData a, glm::mat4x4 viewProjection)
{
	float pitch = a.pitch / 180.0f * glm::pi<float>();
	float yaw = a.yaw / 180.0f * glm::pi<float>();
	glm::mat4x4 rotation = glm::rotate(yaw, glm::vec3(0, 1, 0)) * glm::rotate(pitch, glm::vec3(1, 0, 0));
	glm::mat4x4 translation = glm::translate(glm::vec3(0,0,-a.distance));
	float height = a.height;
	float width = float(a.texture.width) / float(a.texture.height) * a.height;
	glm::mat4x4 scale = glm::scale(glm::vec3(width, height, 1));

	shader.SetMatrixUniform("MVP", viewProjection*rotation*translation*scale);
	shader.BindTexture("image", THUMB_TX_SLOT, a.texture.id);
	Render_DrawModel(quad);
}

void Annotations::Display(glm::mat4x4 projection, glm::mat4x4 view, unsigned int eye, bool showAlignementTool)
{
	shader.Bind();
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (unsigned int i = 0; i < annotations.size(); ++i) {
		if (annotations[i].texture.id) {
			renderAnnotation(annotations[i], projection*view);
		}
	}

	if (showAlignementTool)
	{
		// Render a small black quad on the screen
		glm::mat4x4 translation = glm::translate(glm::vec3(0, 0, -1000.0f));
		float scaleAmount = 10.0f;
		glm::mat4x4 scale = glm::scale(glm::vec3(scaleAmount, scaleAmount, scaleAmount));
		shader.SetMatrixUniform("MVP", projection*translation*scale);
		shader.BindTexture("image", THUMB_TX_SLOT, 0);
		Render_DrawModel(quad);
	}
}