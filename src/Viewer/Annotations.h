#include "glm\glm.hpp"
#include "Render.h"
#include "PanoInfo.h"
#include "Shader.h"

class Annotations
{
public:
	Annotations();
	void Create();
	void Load(std::string annotationsJSONAddress, std::string languageFolder);
	void Display(glm::mat4x4 projection, glm::mat4x4 view, Shader* shader, unsigned int eye, bool showAlignementTool);
	void DisplayHelp(float aspect);
private:
	struct AnnotationData
	{
		std::string filePath;
		Texture texture;
		float yaw = 0;
		float pitch = 0;
		float distance = 0;
		float height = 0;
	};

	Model quad;
	std::vector<AnnotationData> annotations;

	std::vector<AnnotationData> parseAnnotationJSON(std::string jsonText, std::string baseURL);
	void renderAnnotation(AnnotationData a, glm::mat4x4 viewProjection, Shader* shader);
};