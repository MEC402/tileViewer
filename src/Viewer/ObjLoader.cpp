#include "ObjLoader.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "stb_image.h"

ObjLoader::ObjLoader()
{
	//shader.CreateProgram(NULL, "obj.vert", "obj.frag");
}

bool ObjLoader::LoadObj(const char *path, ObjData &o)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::map<std::string, GLuint> textures;

	std::string err;
	bool valid = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path, NULL, false);

	if (!err.empty())
		fprintf(stderr, "Error loading .obj file: %s\n", err);

	if (!valid) {
		fprintf(stderr, "Invalid .obj parse\n");
		return false;
	}

	//printf("# of vertices  = %d\n", (int)(attrib.vertices.size()) / 3);
	//printf("# of normals   = %d\n", (int)(attrib.normals.size()) / 3);
	//printf("# of texcoords = %d\n", (int)(attrib.texcoords.size()) / 2);
	//printf("# of materials = %d\n", (int)materials.size());
	//printf("# of shapes    = %d\n", (int)shapes.size());

	materials.push_back(tinyobj::material_t()); // Default material
	loadTextures(materials, textures);

	std::vector<float> diffuse;
	std::vector<std::vector<float>> vertices(3);
	std::vector<std::vector<float>> txCoords(3);
	std::vector<std::vector<float>> normals(3);


	for (size_t shape = 0; shape < shapes.size(); shape++) {
		ObjData obj;
		std::vector<float> buffer;

		std::map<int, glm::vec3> smoothVertexNormals;
		if (hasSmoothing(shapes[shape])) {
			computeSmoothingNormals(attrib, shapes[shape], smoothVertexNormals);
		}

		// Build triangles
		for (size_t face = 0; face < shapes[shape].mesh.indices.size() / 3; face++) {
			tinyobj::index_t idx[3] = {
				shapes[shape].mesh.indices[face * 3 + 0],
				shapes[shape].mesh.indices[face * 3 + 1],
				shapes[shape].mesh.indices[face * 3 + 2],
			};

			int cur_material_id = shapes[shape].mesh.material_ids[face];

			if (cur_material_id < 0 || cur_material_id >(int)materials.size()) {
				cur_material_id = materials.size() - 1;
			}

			for (size_t i = 0; i < 3; i++) {
				diffuse.push_back(materials[cur_material_id].diffuse[i]);
			}

			loadTexCoords(attrib, idx, txCoords);
			loadVertices(attrib, idx, vertices);
			loadNormals(attrib, idx, normals, smoothVertexNormals, vertices);			
		}

		o.Normals = !normals[0].empty();
		std::vector<float> color;
		std::vector<float> vertex_Ordered;
		std::vector<float> normal_Ordered;
		std::vector<float> txCoord_Ordered;

		// OBJ files get loaded in a different order than is useful for VBOs, so rearrange them
		for (int k = 0, j = 0; k < shapes[shape].mesh.indices.size(); k += 3, j += 2) {
			for (int i = 0; i < 3; i++) {
				vertex_Ordered.push_back(vertices[i][k+0]);
				vertex_Ordered.push_back(vertices[i][k+1]);
				vertex_Ordered.push_back(vertices[i][k+2]);
				if (o.Normals) {
					normal_Ordered.push_back(normals[i][k + 0]);
					normal_Ordered.push_back(normals[i][k + 1]);
					normal_Ordered.push_back(normals[i][k + 2]);

					float normal_factor = 0.2;
					float diffuse_factor = 1 - normal_factor;
					color.push_back(normals[i][k + 0] * normal_factor + diffuse[k + 0] * diffuse_factor);
					color.push_back(normals[i][k + 1] * normal_factor + diffuse[k + 1] * diffuse_factor);
					color.push_back(normals[i][k + 2] * normal_factor + diffuse[k + 2] * diffuse_factor);
				}
				else {
					float tempcolor[] = { 1.0f, 0.0f, 0.0f };
					float len = tempcolor[0] * tempcolor[0] + tempcolor[1] * tempcolor[1] + tempcolor[2] * tempcolor[2];
					if (len > 0.0f) {
						float len2 = sqrtf(len);
						tempcolor[0] /= len2;
						tempcolor[1] /= len2;
						tempcolor[2] /= len2;
					}
					color.push_back(tempcolor[0] * 0.5 + 0.5);
					color.push_back(tempcolor[1] * 0.5 + 0.5);
					color.push_back(tempcolor[2] * 0.5 + 0.5);
				}

				txCoord_Ordered.push_back(txCoords[i][j+0]);
				txCoord_Ordered.push_back(txCoords[i][j+1]);
			}
		}

		o.numTriangles = 0;
		glGenBuffers(1, &o.vertex_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, o.vertex_VBO);
		glBufferData(GL_ARRAY_BUFFER, vertex_Ordered.size() * sizeof(float),
			&vertex_Ordered.front(), GL_STATIC_DRAW);

		if (o.Normals) {
			glGenBuffers(1, &o.normal_VBO);
			glBindBuffer(GL_ARRAY_BUFFER, o.normal_VBO);
			glBufferData(GL_ARRAY_BUFFER, normal_Ordered.size() * sizeof(float),
				&normal_Ordered.front(), GL_STATIC_DRAW);
		}

		glGenBuffers(1, &o.uv_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, o.uv_VBO);
		glBufferData(GL_ARRAY_BUFFER, txCoord_Ordered.size() * sizeof(float),
			&txCoord_Ordered.front(), GL_STATIC_DRAW);

		glGenBuffers(1, &o.color_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, o.color_VBO);
		glBufferData(GL_ARRAY_BUFFER, color.size() * sizeof(float),
			&color.front(), GL_STATIC_DRAW);


		if (shapes[shape].mesh.material_ids.size() > 0 && 
			shapes[shape].mesh.material_ids.size() > shape) {
			o.material_id = shapes[shape].mesh.material_ids[0];
		}
		else {
			o.material_id = materials.size() - 1;
		}

		o.NormalOffset = (o.Normals) ? 0 : 3;
		o.ColorOffset = o.NormalOffset + 3;
		o.TextureOffset = o.ColorOffset + 3;
		o.Stride = (o.Normals) ? 8 : 11;

		o.numTriangles = vertex_Ordered.size() / 3;
	}
	return true;
}

void ObjLoader::DrawObj(ObjData &o, Shader *shader, glm::mat4 ViewProjection)
{
	static float rotate = 0.0f;
	glEnable(GL_DEPTH_TEST);
	shader->Bind();

	double scaling = 0.01;
	glm::mat4 translation = glm::translate(glm::vec3(0.25, 0, 0));
	glm::mat4 rotation = glm::rotate(glm::radians(rotate), glm::vec3(0, 1, 0));
	glm::mat4 scale = glm::scale(glm::vec3(scaling, scaling, scaling));

	glm::mat4 model = translation * rotation * scale;

	shader->SetMatrixUniform("MVP", ViewProjection * model);
	shader->SetMatrixUniform("Model", model);

	rotate += 0.1f;
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, o.vertex_VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (o.Normals) {
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, o.normal_VBO);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, o.color_VBO);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, o.uv_VBO);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, o.numTriangles);
}


void ObjLoader::loadVertices(tinyobj::attrib_t & attrib, tinyobj::index_t idx[], 
	std::vector<std::vector<float>>& vertices)
{
	for (int i = 0; i < 3; i++) {
		int f0 = idx[0].vertex_index;
		int f1 = idx[1].vertex_index;
		int f2 = idx[2].vertex_index;
		if (f0 < 0 || f1 < 0 || f2 < 0) {
			PRINT_GENERIC_ERROR((char*)"No valid verticies found!\n");
			return;
		}

		vertices[0].push_back(attrib.vertices[f0 * 3 + i]);
		vertices[1].push_back(attrib.vertices[f1 * 3 + i]);
		vertices[2].push_back(attrib.vertices[f2 * 3 + i]);
	}
}

void ObjLoader::loadTexCoords(tinyobj::attrib_t &attrib, tinyobj::index_t idx[], 
	std::vector<std::vector<float>> &txCoords)
{
	bool valid = attrib.texcoords.size() > 0;
	if (valid) {
		for (int i = 0; i < 3; i++) {
			if (idx[i].texcoord_index < 0) {
				valid = false;
				break;
			}

			txCoords[i].push_back(attrib.texcoords[2 * idx[i].texcoord_index]);
			txCoords[i].push_back(1.0f - attrib.texcoords[2 * idx[0].texcoord_index + 1]);
		}
	}
	if (!valid) {
		for (int i = 0; i < 3; i++) {
			txCoords[i].push_back(0.0f);
			txCoords[i].push_back(0.0f);
		}
	}
}

void ObjLoader::loadNormals(tinyobj::attrib_t &attrib, tinyobj::index_t idx[], 
	std::vector<std::vector<float>> &normals, std::map<int, glm::vec3> &smoothVertexNormals,
	std::vector<std::vector<float>> &vertices)
{
	bool invalid_normal = false;
	if (attrib.normals.size() > 0) {
		int n0 = idx[0].normal_index;
		int n1 = idx[1].normal_index;
		int n2 = idx[2].normal_index;
		if (n0 < 0 || n1 < 0 || n2 < 0) {
			invalid_normal = true;
		}
		else {
			for (int i = 0; i < 3; i++) {
				normals[0].push_back(attrib.normals[n0 * 3 + i]);
				normals[1].push_back(attrib.normals[n1 * 3 + i]);
				normals[2].push_back(attrib.normals[n2 * 3 + i]);
			}
		}
	}
	else {
		invalid_normal = true;
	}

	if (invalid_normal && !smoothVertexNormals.empty()) {
		// Smoothing normals
		int sn0 = idx[0].vertex_index;
		int sn1 = idx[1].vertex_index;
		int sn2 = idx[2].vertex_index;

		if (sn0 >= 0 && sn1 >= 0 && sn2 >= 0) {
			normals[0].push_back(smoothVertexNormals[sn0].x);
			normals[0].push_back(smoothVertexNormals[sn0].y);
			normals[0].push_back(smoothVertexNormals[sn0].z);

			normals[1].push_back(smoothVertexNormals[sn1].x);
			normals[1].push_back(smoothVertexNormals[sn1].y);
			normals[1].push_back(smoothVertexNormals[sn1].z);

			normals[2].push_back(smoothVertexNormals[sn2].x);
			normals[2].push_back(smoothVertexNormals[sn2].y);
			normals[2].push_back(smoothVertexNormals[sn2].z);

			invalid_normal = false;
		}
	}

	if (invalid_normal) {
		int lastIndex = vertices[0].size();
		float v10[3];
		v10[0] = vertices[1][lastIndex - 3] - vertices[0][lastIndex - 3];
		v10[1] = vertices[1][lastIndex - 2] - vertices[0][lastIndex - 2];
		v10[2] = vertices[1][lastIndex - 1] - vertices[0][lastIndex - 1];

		float v20[3];
		v20[0] = vertices[2][lastIndex - 3] - vertices[0][lastIndex - 3];
		v20[1] = vertices[2][lastIndex - 2] - vertices[0][lastIndex - 2];
		v20[2] = vertices[2][lastIndex - 1] - vertices[0][lastIndex - 1];

		float N[3];
		N[0] = v20[1] * v10[2] - v20[2] * v10[1];
		N[1] = v20[2] * v10[0] - v20[0] * v10[2];
		N[2] = v20[0] * v10[1] - v20[1] * v10[0];

		float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
		if (len2 > 0.0f) {
			float len = sqrtf(len2);

			N[0] /= len;
			N[1] /= len;
			N[2] /= len;
		}
		for (int i = 0; i < 3; i++) {
			normals[i].push_back(N[0]);
			normals[i].push_back(N[1]);
			normals[i].push_back(N[2]);
		}
	}
}

bool ObjLoader::fileExists(const std::string &filename)
{
	FILE *fp;
	fopen_s(&fp, filename.c_str(), "rb");

	if (fp) {
		fclose(fp);
		return true;
	}
	return false;
}

bool ObjLoader::hasSmoothing(tinyobj::shape_t &shape) 
{
	for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
		if (shape.mesh.smoothing_group_ids[i] > 0) {
			return true;
		}
	}
	return false;
}

void ObjLoader::computeSmoothingNormals(tinyobj::attrib_t & attrib, tinyobj::shape_t & shape, 
	std::map<int, glm::vec3>& smoothVertexNormals)
{
	smoothVertexNormals.clear();
	std::map<int, glm::vec3>::iterator iter;

	for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
		// Get the three indexes of the face (all faces are triangular)
		tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
		tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
		tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

		// Get the three vertex indexes and coordinates
		int vi[3];      // indexes
		float v[3][3];  // coordinates

		for (int k = 0; k < 3; k++) {
			vi[0] = idx0.vertex_index;
			vi[1] = idx1.vertex_index;
			vi[2] = idx2.vertex_index;
			assert(vi[0] >= 0);
			assert(vi[1] >= 0);
			assert(vi[2] >= 0);

			v[0][k] = attrib.vertices[3 * vi[0] + k];
			v[1][k] = attrib.vertices[3 * vi[1] + k];
			v[2][k] = attrib.vertices[3 * vi[2] + k];
		}

		// Compute the normal of the face
		float normal[3];
		computeNormals(normal, v[0], v[1], v[2]);

		// Add the normal to the three vertexes
		for (size_t i = 0; i < 3; ++i) {
			iter = smoothVertexNormals.find(vi[i]);
			if (iter != smoothVertexNormals.end()) {
				// add
				iter->second.x += normal[0];
				iter->second.y += normal[1];
				iter->second.z += normal[2];
			}
			else {
				smoothVertexNormals[vi[i]].x = normal[0];
				smoothVertexNormals[vi[i]].y = normal[1];
				smoothVertexNormals[vi[i]].z = normal[2];
			}
		}

	}  // f

	   // Normalize the normals, that is, make them unit vectors
	for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
		iter++) {
		iter->second = glm::normalize(iter->second);
	}

}

void ObjLoader::computeNormals(float N[3], float v0[3], float v1[3], float v2[3]) {
	float v10[3];
	v10[0] = v1[0] - v0[0];
	v10[1] = v1[1] - v0[1];
	v10[2] = v1[2] - v0[2];

	float v20[3];
	v20[0] = v2[0] - v0[0];
	v20[1] = v2[1] - v0[1];
	v20[2] = v2[2] - v0[2];

	N[0] = v20[1] * v10[2] - v20[2] * v10[1];
	N[1] = v20[2] * v10[0] - v20[0] * v10[2];
	N[2] = v20[0] * v10[1] - v20[1] * v10[0];

	float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
	if (len2 > 0.0f) {
		float len = sqrtf(len2);

		N[0] /= len;
		N[1] /= len;
		N[2] /= len;
	}
}

bool ObjLoader::loadTextures(std::vector<tinyobj::material_t> &materials
	,std::map<std::string, GLuint> &textures)
{
	for (size_t m = 0; m < materials.size(); m++) {
		tinyobj::material_t *mPtr = &materials[0];

		if (mPtr->diffuse_texname.length() < 1)
			continue;

		if (textures.find(mPtr->diffuse_texname) != textures.end())
			continue;

		GLuint txID;
		int width, height, nrChannels;
		std::string txName = mPtr->diffuse_texname;
		if (!fileExists(txName)) {
			char buf[128];
			sprintf_s(buf, 128, "Could not locate texture %s\n", mPtr->diffuse_texname);
			PRINT_GENERIC_ERROR(buf);
			return false;
		}

		unsigned char *img = stbi_load(txName.c_str(), &width, &height, &nrChannels, 0);
		if (!img) {
			char buf[128];
			sprintf_s(buf, 128, "Could not load texture %s\n", txName.c_str());
			PRINT_GENERIC_ERROR(buf);
			return false;
		}
#ifdef DEBUG
		fprintf(stderr, "Loaded textures %s\n", txName.c_str());
#endif
		glGenTextures(1, &txID);
		glBindTexture(GL_TEXTURE_2D, txID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(img);
		textures.insert(std::make_pair(mPtr->diffuse_texname, txID));
	}
	return true;
}