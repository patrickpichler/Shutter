#pragma once
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>
#include <array>
#include <vector>
#include "tiny_obj_loader.h"
#include "Renderer/DeviceHandler.h"
#include "Renderer/Buffer.h"

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
	glm::vec3 tangent;
	glm::vec3 bitangent;

	static vk::VertexInputBindingDescription GetBindingDescription() {
		return vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex);
	}

	static std::array<vk::VertexInputAttributeDescription, 5> GetAttributeDescriptions() {
		std::array<vk::VertexInputAttributeDescription, 5> attributeDescriptions;

		attributeDescriptions[0] = vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position));
		attributeDescriptions[1] = vk::VertexInputAttributeDescription(0, 1, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal));
		attributeDescriptions[2] = vk::VertexInputAttributeDescription(0, 2, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord));
		attributeDescriptions[3] = vk::VertexInputAttributeDescription(0, 3, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, tangent));
		attributeDescriptions[4] = vk::VertexInputAttributeDescription(0, 4, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, bitangent));

		return attributeDescriptions;
	}
};

class Mesh {
public:
	Mesh(){}
	Mesh(Device *device);
	void Load(const tinyobj::shape_t &shape, const tinyobj::attrib_t attrib);

	void Clean();

public:
	std::vector<Vertex> _Vertices;
	std::vector<uint32_t> _Indices;

	Buffer _VertexBuffer;

private:
	Device *_Device;

	void GenerateTangents();
};