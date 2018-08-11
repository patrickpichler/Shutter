#include "Object.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Renderer/Helpers.h"

Object::Object(const Device &device, const Mesh &mesh, std::shared_ptr<Material> material, const uint32_t nbImages) :
	_Mesh(mesh),
	_Material(material),
	_NbImages(nbImages)
{
}

void Object::AddTexture(const uint32_t binding, const Texture &texture)
{
	_Textures.emplace(binding, texture);
}

void Object::CreateDescriptorSet(const Device &device)
{
	std::vector<VkDescriptorSetLayout> layouts(_NbImages, _Material->GetDescriptorSetLayout());
	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool =_Material->GetDescriptorPool();
	allocInfo.descriptorSetCount = _NbImages;
	allocInfo.pSetLayouts = layouts.data();

	_DescriptorSets.resize(_NbImages);
	vk_expect_success(vkAllocateDescriptorSets(device.GetLogicalDevice(), &allocInfo, _DescriptorSets.data()), "Failed to allocae descriptor set.");

	size_t i = 0;
	for (const auto &descSet : _DescriptorSets) {
		std::vector<VkWriteDescriptorSet> descriptorWrites;

		VkDescriptorBufferInfo bufferDynamicInfo = {};
		bufferDynamicInfo.buffer = DynamicBuffer.GetBuffer();
		bufferDynamicInfo.offset = 0;
		bufferDynamicInfo.range = sizeof(uboDynamic);

		VkWriteDescriptorSet tempDynamicDesc = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		tempDynamicDesc.dstSet = descSet;
		tempDynamicDesc.dstBinding = 0;
		tempDynamicDesc.dstArrayElement = 0;
		tempDynamicDesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		tempDynamicDesc.descriptorCount = 1;
		tempDynamicDesc.pBufferInfo = &bufferDynamicInfo;

		descriptorWrites.push_back(tempDynamicDesc);

		std::vector<VkDescriptorImageInfo> imageInfos(_Textures.size());
		size_t j = 0;
		for (const auto &texture : _Textures) {
			imageInfos[j] = {};
			imageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			imageInfos[j].imageView = texture.second.GetImage().GetImageView();
			imageInfos[j].sampler = texture.second.GetSampler();

			VkWriteDescriptorSet tempDesc = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
			tempDesc.dstSet = descSet;
			tempDesc.dstBinding = texture.first;
			tempDesc.dstArrayElement = 0;
			tempDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			tempDesc.descriptorCount = 1;
			tempDesc.pImageInfo = &imageInfos[j];

			descriptorWrites.push_back(tempDesc);
			j++;
		}

		vkUpdateDescriptorSets(device.GetLogicalDevice(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
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