#include "Mesh.h"
#include "Renderer/Helpers.h"

Mesh::Mesh(Device * device) : 
	_Device(device)
{
}

void Mesh::Load(const tinyobj::shape_t &shape, const tinyobj::attrib_t attrib)
{
	for (const auto& index : shape.mesh.indices) {
		Vertex vertex = {};

		vertex.position = {
			attrib.vertices[3 * index.vertex_index + 0],
			attrib.vertices[3 * index.vertex_index + 1],
			attrib.vertices[3 * index.vertex_index + 2]
		};

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
