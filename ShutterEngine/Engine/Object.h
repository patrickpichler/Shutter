#pragma once

#include <map>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>
#include "Mesh.h"
#include "Texture.h"
#include "Renderer/Material.h"
#include "Engine/SceneObject.h"

class Object : public SceneObject {
public:
	Object(){}
	explicit Object(Device *device, const Mesh &mesh, Material *material, const uint32_t nbImages);

	void AddTexture(const uint32_t binding, const Texture &texture);

	void CreateDescriptorSet();

	Material *GetMaterial() {
		return _Material;
	}
	const vk::DescriptorSet &GetDescriptorSet(const uint32_t id) const {
		return _DescriptorSets.at(id);
	}

	const glm::mat4 GetModelMatrix() const;

private:
	Material *_Material;


public:
	Mesh _Mesh;
	glm::vec3 _Rotation;
	glm::vec3 _Scale;
	uint32_t _DynamicIndex;

	static Buffer DynamicBuffer;
	static uint32_t dynamicAlignement;

	static struct UboDynamic {
		glm::mat4 *model = nullptr;
	} uboDynamic;
private:

	Device *_Device;

	uint32_t _NbImages;

	// Map containing the relation between a texture and its binding
	std::map<uint32_t, Texture> _Textures;


	std::vector<vk::DescriptorSet> _DescriptorSets;
};