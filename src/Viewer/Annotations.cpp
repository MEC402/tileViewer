#include <GL\glew.h>
#include <GL\freeglut.h> // Used to draw text

#include "Annotations.h"
#include "InternetDownload.h"
#include "stb_image.h"
#include "Shared.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "include/wkhtmltox/image.h"

Annotations::Annotations()
{
	
}

void Annotations::Create()
{
	Render::CreateQuadModel(&quad);
}

void Annotations::Load(std::string annotationsJSONAddress, std::string languageFolder)
{
	// Destroy any existing annotations
	for (unsigned int i = 0; i < annotations.size(); ++i) {
		Render::DestroyTexture(&annotations[i].texture);
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
	char buf[512];
	for (unsigned int i = 0; i < annotations.size(); ++i) {
		
		urls.push_back(buf);
	}
	//std::vector<ImageData> files = downloadMultipleFiles(urls.data(), urls.size());

	// Create a texture for each image
	for (unsigned int i = 0; i< annotations.size(); ++i)
	{
		// Test load from html page
		wkhtmltoimage_init(true);
		wkhtmltoimage_global_settings* settings = wkhtmltoimage_create_global_settings();
		settings = wkhtmltoimage_create_global_settings();

		sprintf_s(buf, annotations[i].filePath.c_str(), languageFolder.c_str());
		wkhtmltoimage_set_global_setting(settings, "in", buf);
		wkhtmltoimage_set_global_setting(settings, "fmt", "png");
		wkhtmltoimage_set_global_setting(settings, "out", ""); // Write to an internal buffer
		float pixelsPerMeter = 50;
		sprintf_s(buf, "%d", int(annotations[i].width*pixelsPerMeter));
		wkhtmltoimage_set_global_setting(settings, "screenWidth", buf);
		wkhtmltoimage_converter* converter = wkhtmltoimage_create_converter(settings, 0);
		wkhtmltoimage_convert(converter);
		const unsigned char* outputImage = 0;
		long outputSize = wkhtmltoimage_get_output(converter, &outputImage);

		if (outputSize) {
			int width, height, nrChannels;
			//unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)files[i].data, files[i].dataSize, &width, &height, &nrChannels, 0));
			unsigned char* d = (unsigned char*)(stbi_load_from_memory((stbi_uc*)outputImage, outputSize, &width, &height, &nrChannels, 0));

			GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
			Render::CreateTexture(&annotations[i].texture, THUMB_TX_SLOT, width, height, format, d);

			stbi_image_free(d);
		}

		wkhtmltoimage_destroy_converter(converter);
		wkhtmltoimage_deinit();
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
			if (annotationsArray[i].HasMember("width")) {
				a.width = annotationsArray[i]["width"].GetFloat();
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

void Annotations::renderAnnotation(AnnotationData a, glm::mat4x4 viewProjection, Shader* shader)
{
	float pitch = a.pitch / 180.0f * glm::pi<float>();
	float yaw = a.yaw / 180.0f * glm::pi<float>();
	glm::mat4x4 rotation = glm::rotate(yaw, glm::vec3(0, 1, 0)) * glm::rotate(pitch, glm::vec3(1, 0, 0));
	glm::mat4x4 translation = glm::translate(glm::vec3(0,0,-a.distance));
	float width = a.width;
	float height = float(a.texture.height) / float(a.texture.width) * a.width;
	glm::mat4x4 scale = glm::scale(glm::vec3(width, height, 1));

	shader->SetMatrixUniform("MVP", viewProjection*rotation*translation*scale);
	shader->BindTexture("image", THUMB_TX_SLOT, a.texture.id);
	Render::DrawModel(quad);
}

void Annotations::Display(glm::mat4x4 projection, glm::mat4x4 view, Shader* shader, unsigned int eye, bool showAlignementTool)
{
	shader->Bind();
	glDisable(GL_DEPTH_TEST);
	// Enable alpha blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (unsigned int i = 0; i < annotations.size(); ++i) {
		if (annotations[i].texture.id) {
			renderAnnotation(annotations[i], projection*view, shader);
		}
	}

	if (showAlignementTool)
	{
		// Render a small black quad on the screen
		glm::mat4x4 translation = glm::translate(glm::vec3(0, 0, -1000.0f));
		float scaleAmount = 10.0f;
		glm::mat4x4 scale = glm::scale(glm::vec3(scaleAmount, scaleAmount, scaleAmount));
		shader->SetMatrixUniform("MVP", projection*translation*scale);
		shader->BindTexture("image", THUMB_TX_SLOT, 0);
		Render::DrawModel(quad);
	}
}

// TODO: Don't blank out the entire screen, show some kind of nice overlay (i.e. An Annotation)
// TODO: Doesn't work on 5-panel displays
void Annotations::DisplayHelp(float aspect)
{
	// Disable shader program temporarily, it's just way way easier to call glutBitmap
	// than rendering everything as textures onto quads for text
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, aspect, 0.1f, 1000.0f);

	gluLookAt(0, 0, 0,
		0, 0, 1,
		0, 1, 0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	const char *help[] = { "COMMANDS", 
		"wasd/Arrow Keys - Rotate Up/Down/Right/Left", "WASD - Rotate faster", 
		"e - Set FOV to 1:1 Pixel Matching on the screen", 
		"r - Reset camera Pitch and FOV", "R - Reload current Panorama",
		"f - Toggle Fullscreen",
		"C - Toggle Comparison Mode. Select the pano to compare against, then use 1/2 to toggle between them.", 
		"h - Toggle horizontal split Stereo mode",
		"n - Next Pano (also applies inside GUI selection)",
		"p - Previous Pano (also applies inside GUI selection)",
		"L - Toggle Linear/Nearest Texture Filtering",
		"F1 - Display this message",
		"F2/F3 - Reload Shaders",
		"F4 - Display GUI Pano Selection",
		"F5 - Display Annotations",
		"F8 - Toggle \"Texture Debug View\"",
		"F9 - Take a screenshot",
		"Scroll Wheel - Zoom in/out",
		"PG Up/Down - Zoom in/out slowly",
		"ESC - Exit Program"};
	int commandCount = sizeof(help)/sizeof(help[0]);

	glScalef(0.025, 0.025, 1.0);

	glLineWidth(1.5);
	glColor3f(0.0, 1.0, 0.0);
	for (int i = 0; i < commandCount; i++) {
		glRasterPos3i(25, 10-i, 0.2);
		for (int j = 0; j < strlen(help[i]); j++) {
			glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, help[i][j]);
		}
	}
}