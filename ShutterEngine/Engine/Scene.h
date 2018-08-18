#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Renderer/DeviceHandler.h"
#include "Renderer/Buffer.h"

#include "Light.h"
#include "Renderer/Material.h"
#include "Renderer/Cubemap.h"
#include "Engine/Mesh.h"
#include "Object.h"
#include "Texture.h"
#include "CubeTexture.h"

struct SceneDataObject {
	union alignas(256) Data{
		CameraUniformData _CameraData;
		LightUniformData _LightData[2];
	} _Data[2];
};

class Scene {
public:
	// Load a scene from a set of yaml files
	void Load(const std::string &name, Device *device, const vk::CommandPool &cmdPool, const vk::RenderPass &renderPass);


	std::vector<Light> _Lights;
	std::unordered_map<std::string, Material> _Materials;
	std::unordered_map<std::string, Cubemap> _Cubemaps;
	std::unordered_map<std::string, Mesh> _Models;
	std::unordered_map<std::string, std::vector<Object>> _Objects;
	std::unordered_map<std::string, Texture> _Textures;

	void CreateDynamic(Device *device);
	uint32_t AddToDynamic(const Object &object);
	void UploadDynamic();




	void CreateDescriptorSets(Device *device, const uint32_t nbImages);
	void Update(const uint32_t image);


	vk::DescriptorSet GetDescriptorSet(const uint32_t image) const {
		return _SceneDescriptorSets.at(image);
	}

	vk::DescriptorSetLayout GetDescriptorSetLayout() const {
		return _DescriptorSetLayout;
	}

private:
	void CreateDescriptorSetLayout(const uint32_t nbImages);

public:
	Camera _Camera;
	std::string _Name;

private:
	Device *_Device;

	size_t dynamicIndex = 0;

	vk::DescriptorPool _DescriptorPool;

	std::vector<vk::DescriptorSet> _SceneDescriptorSets;

	vk::DescriptorSetLayout _DescriptorSetLayout;

	std::vector<Buffer> _SceneDataBuffers;
	std::vector<SceneDataObject> _SceneDataObjects;
};