#include "Scene.h"

void Scene::CreateDescriptorSets(Device *device, const uint32_t nbImages)
{
	_Device = device;

	CreateDescriptorSetLayout();

	_SceneDescriptorSets.resize(nbImages);
	_SceneDataBuffers.resize(nbImages);
	_SceneDataObjects.resize(nbImages);

	for (size_t i = 0; i < nbImages; ++i)
	{
		_SceneDataBuffers.at(i) = Buffer(*_Device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(SceneDataObject));

		vk::DescriptorBufferInfo cameraInfo = {
			_SceneDataBuffers.at(i).GetBufferNew(),
			0,
			sizeof(SceneDataObject::CameraData)
		};

		vk::WriteDescriptorSet cameraDescriptor(
			_SceneDescriptorSets.at(i),
			0, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&cameraInfo
		);

		vk::DescriptorBufferInfo lightInfo = {
			_SceneDataBuffers.at(i).GetBufferNew(),
			sizeof(SceneDataObject::CameraData),
			sizeof(SceneDataObject::LightData)
		};

		vk::WriteDescriptorSet lightDescriptor(
			_SceneDescriptorSets.at(i),
			1, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&lightInfo
		);

		_Device->GetLogicalDeviceNew().updateDescriptorSets({ cameraDescriptor, lightDescriptor }, nullptr);
	}
}

void Scene::Update(const uint32_t image)
{
	// Prepare the camera
	_SceneDataObjects.at(image)._CameraData._Position = glm::vec3(0.0f, 0.0f, 0.0f);
	_SceneDataObjects.at(image)._CameraData._Projection = _Camera->GetProjection();
	_SceneDataObjects.at(image)._CameraData._View = _Camera->GetView();

	// Add a white light in 0,0,0
	_SceneDataObjects.at(image)._LightData[0]._Position = glm::vec3(.0f, .0f, .0f);
	_SceneDataObjects.at(image)._LightData[0]._Colour = glm::vec3(1.0f, 1.0f, 1.0f);

	_SceneDataBuffers.at(image).Copy(*_Device, &_SceneDataObjects.at(image), sizeof(SceneDataObject));
}

void Scene::CreateDescriptorSetLayout()
{
	vk::DescriptorSetLayoutBinding cameraInfo(0, vk::DescriptorType::eUniformBuffer, 1,  vk::ShaderStageFlagBits::eVertex);
	vk::DescriptorSetLayoutBinding lightInfo(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings{ cameraInfo, lightInfo };


	_DescriptorSetLayout = _Device->GetLogicalDeviceNew().createDescriptorSetLayout({ {}, 1, bindings.data() });
}
