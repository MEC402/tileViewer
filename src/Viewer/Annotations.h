#include "glm\glm.hpp"
#include "Render.h"
#include "PanoInfo.h"
#include "Shader.h"
#include "include/wkhtmltox/image.h"
#include <thread>
#include <mutex>

class Annotations
{
public:
	Annotations();
	void Create(std::string languageFolder);
	void Load(std::string annotationsJSONAddress, std::string languageFolder);
	void Display(glm::mat4x4 projection, glm::mat4x4 view, Shader* shader, unsigned int eye, bool showAlignementTool);
	void DisplayHelp(float aspect);
private:
	struct AnnotationData
	{
		std::string filePath;
		Texture texture;
		unsigned char* imageData =0;
		int imageWidth = 0;
		int imageHeight = 0;
		GLuint imageFormat = 0;
		float yaw = 0;
		float pitch = 0;
		float distance = 0;
		float width = 0;
		float pixelsPerMeter = 0;
		float zoom = 0;
	};

	Model quad;
	std::string languageFolder;
	std::vector<AnnotationData> annotations;
	std::mutex dataMutex;
	std::mutex threadRunning;
	std::condition_variable resumeThread;

	std::vector<AnnotationData> parseAnnotationJSON(std::string jsonText, std::string baseURL);
	void renderAnnotation(AnnotationData a, glm::mat4x4 viewProjection, Shader* shader);
	
	static void Annotations::RenderHTMLPages(Annotations* a);
};