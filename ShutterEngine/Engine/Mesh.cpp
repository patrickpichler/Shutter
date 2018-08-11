#include "Mesh.h"
#include "Renderer/Helpers.h"

void Mesh::Load(const Device &device, const tinyobj::shape_t &shape, const tinyobj::attrib_t attrib)
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

		Vertices.push_back(vertex);
		Indices.push_back(Indices.size());
	}

	GenerateTangents();

	VertexBuffer = Buffer(device, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, sizeof(Vertex) * Vertices.size());
	VertexBuffer.Copy(device, Vertices.data(), sizeof(Vertex) * Vertices.size());
}

void Mesh::Clean(const Device &device)
{
	VertexBuffer.Clean(device);
}

void Mesh::GenerateTangents()
{
	for (int i = 0; i < Vertices.size(); i += 3) {
		glm::vec3 & v0 = Vertices[i + 0].position;
		glm::vec3 & v1 = Vertices[i + 1].position;
		glm::vec3 & v2 = Vertices[i + 2].position;

		// Shortcuts for UVs
		glm::vec2 & uv0 = Vertices[i + 0].texCoord;
		glm::vec2 & uv1 = Vertices[i + 1].texCoord;
		glm::vec2 & uv2 = Vertices[i + 2].texCoord;

		// Edges of the triangle : position delta
		glm::vec3 deltaPos1 = v1 - v0;
		glm::vec3 deltaPos2 = v2 - v0;

		// UV delta
		glm::vec2 deltaUV1 = uv1 - uv0;
		glm::vec2 deltaUV2 = uv2 - uv0;

		float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		Vertices[i + 0].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		Vertices[i + 0].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
		Vertices[i + 1].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		Vertices[i + 1].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
		Vertices[i + 2].tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y)*r;
		Vertices[i + 2].bitangent = (deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x)*r;
	}
}
