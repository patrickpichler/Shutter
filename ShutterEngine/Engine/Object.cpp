#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Helpers.h"

Object::Object(Device *device, const Mesh &mesh, Material *material, const uint32_t nbImages) :
	_Device(device),
	_Mesh(mesh),
	_Material(material),
	_NbImages(nbImages)
{
}

void Object::AddTexture(const uint32_t binding, const Texture &texture)
{
	_Textures.emplace(binding, texture);
}

void Object::CreateDescriptorSet()
{
	std::vector<vk::DescriptorSetLayout> layouts(_NbImages, _Material->GetDescriptorSetLayout());

	_DescriptorSets = _Device->GetDevice().allocateDescriptorSets(vk::DescriptorSetAllocateInfo(
		_Material->GetDescriptorPool(),
		_NbImages,
		layouts.data()
	));

	size_t i = 0;
	for (const auto &descSet : _DescriptorSets) {
		std::vector<vk::WriteDescriptorSet> descriptorWrites;

		// Model Matrix
		descriptorWrites.push_back(vk::WriteDescriptorSet(
			descSet,
			0,
			0,
			1,
			vk::DescriptorType::eUniformBufferDynamic,
			nullptr,
			&vk::DescriptorBufferInfo(
				DynamicBuffer.GetBuffer(),
				0,
				sizeof(uboDynamic)
			),
			nullptr
		));

		size_t j = 0;
		for (const auto &texture : _Textures) {
			descriptorWrites.push_back(vk::WriteDescriptorSet(
				descSet,
				texture.first,
				0,
				1,
				vk::DescriptorType::eCombinedImageSampler,
				&vk::DescriptorImageInfo(
					texture.second.GetSampler(),
					texture.second.GetImage().GetImageView(),
					vk::ImageLayout::eShaderReadOnlyOptimal
				),
				nullptr,
				nullptr
			));
			j++;
		}

		_Device->GetDevice().updateDescriptorSets(descriptorWrites, {});
		i++;
	}
}

const glm::mat4 Object::GetModelMatrix() const
{
	glm::mat4 model;

	model = glm::translate(glm::mat4(1.0f), _Position);
	model = glm::rotate(model, _Rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::rotate(model, _Rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
	model = glm::rotate(model, _Rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, _Scale);

	return model;
}

Buffer Object::DynamicBuffer = Buffer();
uint32_t Object::dynamicAlignement = 0;
Object::UboDynamic Object::uboDynamic = {};