#pragma once

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.h>
#include "Mesh.h"
#include "Texture.h"
#include "Renderer/Material.h"

class Object {
public:
	Object(){}
	Object(const Device &device, const Mesh &mesh, std::shared_ptr<Material> material, const uint32_t nbImages);

	void AddTexture(const uint32_t binding, const Texture &texture);

	void CreateDescriptorSet(const Device &device);

	std::shared_ptr<Material> GetMaterial() {
		return _Material;
	}
	const VkDescriptorSet &GetDescriptorSet(const uint32_t id) const {
		return _DescriptorSets.at(id);
	}

	const glm::mat4 GetModelMatrix() const;

private:
	Mesh _Mesh;
	std::shared_ptr<Material> _Material;


public:
	glm::vec3 _Position;
	glm::vec3 _Rotation;
	glm::vec3 _Scale;

	static Buffer DynamicBuffer;
	static uint32_t dynamicAlignement;

	static struct UboDynamic {
		glm::mat4 *model = nullptr;
	} uboDynamic;
private:

	uint32_t _NbImages;

	// Map containing the relation between a texture and its binding
	std::map<uint32_t, Texture> _Textures;


	std::vector<VkDescriptorSet> _DescriptorSets;
};