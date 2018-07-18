#ifndef _OBJLOADER_H
#define _OBJLOADER_H

#include <string>
#include <vector>
#include "Shader.h"
#include "Shared.h"
#include "glm/glm.hpp"
#include "tiny_obj_loader.h"

class ObjLoader {
public:
	typedef struct {
		GLuint vertex_VBO = 0;  // vertex buffer id
		GLuint normal_VBO = 0;
		GLuint uv_VBO = 0;
		GLuint color_VBO = 0;

		int numTriangles;
		size_t material_id;

		int Stride;
		int NormalOffset;
		int ColorOffset;
		int TextureOffset;

		bool Normals = true;
		bool Color = true;
		bool Texture = true;
	} ObjData;

	ObjLoader();
	bool LoadObj(const char *path, ObjData &o);
	void DrawObj(ObjData &o, Shader *shader, glm::mat4 ModelViewProjection);

private:
	Shader shader;

	bool fileExists(const std::string &filename);
	bool loadTextures(std::vector<tinyobj::material_t> &materials, std::map<std::string, GLuint> &textures);
	void loadVertices(tinyobj::attrib_t &attrib, tinyobj::index_t idx[], std::vector<std::vector<float>> &vertices);
	void loadTexCoords(tinyobj::attrib_t &attrib, tinyobj::index_t idx[], std::vector<std::vector<float>> &txCoords);
	void loadNormals(tinyobj::attrib_t &attrib, tinyobj::index_t idx[], std::vector<std::vector<float>> &normals, std::map<int, glm::vec3> &smoothVertexNormals, std::vector<std::vector<float>> &vertices);

	bool hasSmoothing(tinyobj::shape_t &shape);
	void computeSmoothingNormals(tinyobj::attrib_t &attrib, tinyobj::shape_t &shape, std::map<int, glm::vec3> &smoothVertexNormals);
	void computeNormals(float N[3], float v0[3], float v1[3], float v2[3]);
};

#endif // _OBJLOADER_H