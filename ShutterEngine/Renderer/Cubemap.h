#pragma once

#include <string>
#include "Shader.h"
#include "Material.h"

class Cubemap: public Material {
public:
	Cubemap::Cubemap(const Device &device, const uint16_t width, const uint16_t height, const uint32_t poolSize)
	{
		CreateDescriptorSetLayout(device);
		CreateDescriptorPool(device, poolSize);

		PopulateInfo(width, height);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &_DesciptorSetLayout;

		VkResult result = vkCreatePipelineLayout(device.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &PipelineLayout);
	}
protected:
	void CreateRasterizationInfo() override;
	void CreateDescriptorPool(const Device &device, const uint32_t poolSize) override;
	void CreateDescriptorSetLayout(const Device &device) override;
};