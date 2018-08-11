#include "Cubemap.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

void Cubemap::CreateDescriptorSetLayout(const Device & device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1;
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerBinding = {};
	samplerBinding.binding = 2;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, modelLayoutBinding, samplerBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 3;
	layoutInfo.pBindings = bindings;

	vk_expect_success(vkCreateDescriptorSetLayout(device.GetLogicalDevice(), &layoutInfo, nullptr, &_DesciptorSetLayout), "Failed to create the descriptor set.");
}

void Cubemap::CreateDescriptorPool(const Device & device, const uint32_t poolSize)
{
	VkDescriptorPoolSize poolSizes[3];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = poolSize;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[1].descriptorCount = poolSize;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = poolSize;

	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = 3;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = poolSize;

	vk_expect_success(vkCreateDescriptorPool(device.GetLogicalDevice(), &poolInfo, nullptr, &_DescriptorPool), "Fail to create the desciptor pool.");
}

void Cubemap::CreateRasterizationInfo()
{
	rasterInfo = {};
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.depthClampEnable = VK_FALSE;
	rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterInfo.lineWidth = 1.0f;
	rasterInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterInfo.depthBiasEnable = VK_FALSE;
}
