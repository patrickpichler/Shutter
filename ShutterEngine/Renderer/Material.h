#pragma once

#include <string>
#include <unordered_map>
#include "Shader.h"

class Material {
public:
	Material(){}

	Material(
		const Device &device,
		const uint16_t width,
		const uint16_t height,
		const uint32_t poolSize
	);

	void BindShader(const Shader &shader);

	void CreatePipeline(const Device &device, const VkRenderPass &renderPass);

	const VkDescriptorPool &GetDescriptorPool() const {
		return _DescriptorPool;
	}
	const VkDescriptorSetLayout &GetDescriptorSetLayout() const {
		return _DesciptorSetLayout;
	}
	const VkPipeline &GetPipeline() const {
		return _Pipeline;
	}
	const VkPipelineLayout &GetPipelineLayout() const {
		return PipelineLayout;
	}

protected:

	void PopulateInfo(const uint32_t width, const uint32_t height);

	void CreateInputAssemblyInfo();
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;

	void CreateViewportInfo(const uint32_t width, const uint32_t height);
	VkViewport viewport;
	VkRect2D scissor;
	VkPipelineViewportStateCreateInfo viewportState;

	virtual void CreateRasterizationInfo();
	VkPipelineRasterizationStateCreateInfo rasterInfo;

	void CreateMultisamplingInfo();
	VkPipelineMultisampleStateCreateInfo multisamplingInfo;

	void CreateDepthStencilInfo();
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

	void CreateColorBlendInfo();
	VkPipelineColorBlendAttachmentState colorBlend;
	VkPipelineColorBlendStateCreateInfo colorBlendInfo;

	virtual void CreateDescriptorSetLayout(const Device &device);
	VkDescriptorSetLayout _DesciptorSetLayout;


	virtual void CreateDescriptorPool(const Device &device, const uint32_t poolSize);
	VkPipelineLayout PipelineLayout;

	std::vector<VkPipelineShaderStageCreateInfo> GetShaderInfoList();

//private:
	std::unordered_map<VkShaderStageFlagBits, Shader> ShaderMap;

	VkDescriptorPool _DescriptorPool;

	VkPipeline _Pipeline;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo;
	VkPushConstantRange pushConstantRange[1];
};