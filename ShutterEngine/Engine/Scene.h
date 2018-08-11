#pragma once
#include <vector>
#include <memory>
#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include "Camera.h"
#include "Renderer/DeviceHandler.h"
#include "Renderer/Buffer.h"

#define NB_LIGHTS 1

struct SceneDataObject {
	struct CameraData {
		glm::mat4 _View;
		glm::mat4 _Projection;
		glm::vec3 _Position;
	} _CameraData;

	struct LightData {
		glm::vec3 _Position;
		glm::vec3 _Colour;
	} _LightData[NB_LIGHTS];
};

class Scene {
public:
	void CreateDescriptorSets(Device *device, const uint32_t nbImages);
	void Update(const uint32_t image);


	vk::DescriptorSet GetDescriptorSet(const uint32_t image) const {
		return _SceneDescriptorSets.at(image);
	}

	vk::DescriptorSetLayout GetDescriptorSetLayout() const {
		return _DescriptorSetLayout;
	}

private:
	void CreateDescriptorSetLayout();

public:
	std::shared_ptr<Camera> _Camera;

private:
	Device *_Device;

	std::vector<vk::DescriptorSet> _SceneDescriptorSets;

	vk::DescriptorSetLayout _DescriptorSetLayout;

	std::vector<Buffer> _SceneDataBuffers;
	std::vector<SceneDataObject> _SceneDataObjects;
};