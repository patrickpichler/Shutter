#include "Mesh.h"
#include "Renderer/Helpers.h"
#include <fstream>

Mesh::Mesh(Device * device) : 
	_Device(device)
{
}

std::unordered_map<std::string, Mesh> Mesh::Load(Device *device, const std::string &filename, const std::string &root, const vk::CommandPool &cmdPool)
{
	//std::ofstream file(filename + ".txt");

	std::unordered_map<std::string, Mesh> meshes;

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filename.c_str(), (root + "models/").c_str());

	for (size_t i = 0; i < shapes.size(); ++i) {
		Mesh tempMesh(device);
		tempMesh.Load(shapes.at(i), attrib, cmdPool);



		//file << "  - name: " << shapes[i].name << "\n";
		//file << "    model: " << shapes[i].name << "\n";
		//file << "    material: basic\n";

		//if (shapes.at(i).mesh.material_ids.front() < materials.size()) {
		//	if (materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname != "") {
		//		file << "    textures:\n";
		//		file << "      - slot: 1\n";
		//		std::string path = materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname;
		//		file << "        texture: " << path << "\n";
		//	}
		//}
		//file << "    position: [0.0,0.0,0.0]\n";
		//file << "    rotation: [0.0,0.0,0.0]\n";
		//file << "    scale: [0.1,0.1,0.1]\n";

		meshes.insert(std::pair<std::string, Mesh>(shapes[i].name, tempMesh));
	}

	//file.close();

	return meshes;
}

void Mesh::Load(const tinyobj::shape_t &shape, const tinyobj::attrib_t attrib, const vk::CommandPool &cmdPool)
{
	_BoxMax = glm::vec3(0.0f, 0.0f, 0.0f);
	_BoxMin = glm::vec3(0.0f, 0.0f, 0.0f);

	for (const auto& index : shape.mesh.indices) {
		Vertex vertex = {};

		vertex.position = {
			attrib.vertices[3 * index.vertex_index + 0],
			attrib.vertices[3 * index.vertex_index + 1],
			attrib.vertices[3 * index.vertex_index + 2]
		};

		// Compute bounding box max point
		_BoxMax.x = std::max(_BoxMax.x, vertex.position.x);
		_BoxMax.y = std::max(_BoxMax.y, vertex.position.y);
		_BoxMax.z = std::max(_BoxMax.z, vertex.position.z);

		// Compute bounding box min point
		_BoxMin.x = std::min(_BoxMax.x, vertex.position.x);
		_BoxMin.y = std::min(_BoxMax.y, vertex.position.y);
		_BoxMin.z = std::min(_BoxMax.z, vertex.position.z);

		if (attrib.normals.size() > 0) {
			vertex.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};
		}

		if (attrib.texcoords.size() > 0) {
			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};
		}

		_Vertices.push_back(vertex);
		_Indices.push_back(_Indices.size());
	}

	GenerateTangents();

	Upload(cmdPool);
}

void Mesh::Upload(const vk::CommandPool & cmdPool)
{

	Buffer stagingBuffer = Buffer(_Device, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(Vertex) * _Vertices.size());
	stagingBuffer.Copy(_Vertices.data(), sizeof(Vertex) * _Vertices.size());

	_VertexBuffer = Buffer(_Device, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(Vertex) * _Vertices.size(), vk::SharingMode::eExclusive, vk::MemoryPropertyFlagBits::eDeviceLocal);
	stagingBuffer.Transfer(_VertexBuffer, cmdPool);

	stagingBuffer.Clean();
}

void Mesh::Upload()
{
	_VertexBuffer = Buffer(_Device, vk::BufferUsageFlagBits::eVertexBuffer, sizeof(Vertex) * _Vertices.size());
	_VertexBuffer.Copy(_Vertices.data(), sizeof(Vertex) * _Vertices.size());
}

void Mesh::Clean()
{
	//_VertexBuffer.Clean();
}

void Mesh::GenerateTangents()
{
	for (int i = 0; i < _Vertices.size(); i += 3) {
		glm::vec3 & v0 = _Vertices[i + 0].position;
		glm::vec3 & v1 = _Vertices[i + 1].position;
		glm::vec3 & v2 = _Vertices[i + 2].position;

		// Shortcuts for UVs
		glm::vec2 & uv0 = _Vertices[i + 0].texCoord;
		glm::vec2 & uv1 = _Vertices[i + 1].texCoord;
		glm::vec2 & uv2 = _Vertices[i + 2].texCoord;

		// Edges of the triangle : position delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		_Vertices[i + 0].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		_Vertices[i + 0].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
		_Vertices[i + 1].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		_Vertices[i + 1].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
		_Vertices[i + 2].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		_Vertices[i + 2].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
	}
}
