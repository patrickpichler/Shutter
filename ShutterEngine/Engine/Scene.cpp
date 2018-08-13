#include "Scene.h"

void Scene::CreateDescriptorSets(Device *device, const uint32_t nbImages)
{
	_Device = device;

	CreateDescriptorSetLayout(nbImages);

	std::vector<vk::DescriptorSetLayout> layouts(nbImages, _DescriptorSetLayout);

	_SceneDescriptorSets = _Device->GetDevice().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(
		_DescriptorPool,
		nbImages,
		layouts.data()
	));

	_SceneDescriptorSets.resize(nbImages);
	_SceneDataBuffers.resize(nbImages);
	_SceneDataObjects.resize(nbImages);

	for (size_t i = 0; i < nbImages; ++i)
	{
		_SceneDataBuffers.at(i) = Buffer(_Device, vk::BufferUsageFlagBits::eUniformBuffer, sizeof(SceneDataObject::CameraData));

		vk::WriteDescriptorSet cameraDescriptor(
			_SceneDescriptorSets.at(i),
			0, 0, 1,
			vk::DescriptorType::eUniformBuffer,
			nullptr,
			&vk::DescriptorBufferInfo(
				_SceneDataBuffers[i].GetBuffer(),
				0,
				sizeof(SceneDataObject::CameraData)
			),
			nullptr
		);

		//vk::DescriptorBufferInfo lightInfo = {
		//	_SceneDataBuffers.at(i).GetBuffer(),
		//	sizeof(SceneDataObject::CameraData),
		//	sizeof(SceneDataObject::LightData)
		//};

		//vk::WriteDescriptorSet lightDescriptor(
		//	_SceneDescriptorSets.at(i),
		//	1, 0, 1,
		//	vk::DescriptorType::eUniformBuffer,
		//	nullptr,
		//	&lightInfo
		//);

		_Device->GetDevice().updateDescriptorSets(cameraDescriptor, nullptr);
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

	_SceneDataBuffers.at(image).Copy(&_SceneDataObjects.at(image), sizeof(SceneDataObject));
}

void Scene::CreateDescriptorSetLayout(const uint32_t nbImages)
{
	vk::DescriptorSetLayoutBinding cameraInfo(0, vk::DescriptorType::eUniformBuffer, 1,  vk::ShaderStageFlagBits::eVertex);
	vk::DescriptorSetLayoutBinding lightInfo(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eFragment);

	std::vector<vk::DescriptorSetLayoutBinding> bindings{ cameraInfo /*, lightInfo*/ };


	_DescriptorSetLayout = _Device->GetDevice().createDescriptorSetLayout(
		vk::DescriptorSetLayoutCreateInfo(
			{},
			bindings.size(),
			bindings.data()
		)
	);

	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.resize(bindings.size());

	for (size_t i = 0; i < bindings.size(); ++i) {
		poolSizes[i].type = bindings[i].descriptorType;
		poolSizes[i].descriptorCount = nbImages;
	}

	_DescriptorPool = _Device->GetDevice().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, nbImages, poolSizes.size(), poolSizes.data()));
}