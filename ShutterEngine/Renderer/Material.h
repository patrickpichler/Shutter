#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "Shader.h"

class Scene;

class Material {
public:
	Material() {}
	explicit Material(
		Device *device,
		Scene *scene,
		const uint16_t width,
		const uint16_t height,
		const uint32_t poolSize
	);

	void ReloadPipeline(const vk::RenderPass &renderPass, const uint16_t width, const uint16_t height);

	void BindShader(const Shader &shader);
	void ClearShaders();
	std::vector<Shader> GetShaderList();

	void CreatePipeline(const vk::RenderPass &renderPass);

	const vk::DescriptorPool &GetDescriptorPool() const {
		return _DescriptorPool;
	}
	const vk::DescriptorSetLayout &GetDescriptorSetLayout() const {
		return _DesciptorSetLayout;
	}
	const vk::Pipeline &GetPipeline() const {
		return _Pipeline;
	}
	const vk::PipelineLayout &GetPipelineLayout() const {
		return _PipelineLayout;
	}

protected:
	Device *_Device;
	Scene *_Scene;

	void PopulateInfo(const uint32_t width, const uint32_t height);

	void CreateInputAssemblyInfo();
	vk::PipelineInputAssemblyStateCreateInfo _InputAssemblyInfo;

	void CreateViewportInfo(const uint32_t width, const uint32_t height);
	vk::Viewport _Viewport;
	vk::Rect2D _Scissor;
	vk::PipelineViewportStateCreateInfo _ViewportInfo;

	virtual void CreateRasterizationInfo();
	vk::PipelineRasterizationStateCreateInfo _RasterizationInfo;

	void CreateMultisampleInfo();
	vk::PipelineMultisampleStateCreateInfo _MultisampleInfo;

	void CreateDepthStencilInfo();
	vk::PipelineDepthStencilStateCreateInfo _DepthStencilInfo;

	void CreateColorBlendInfo();
	vk::PipelineColorBlendAttachmentState _ColorBlendAttachement;
	vk::PipelineColorBlendStateCreateInfo _ColorBlendInfo;

	virtual void CreateDescriptorSetLayout();
	std::vector<vk::DescriptorSetLayoutBinding> _LayoutBindings;
	vk::DescriptorSetLayout _DesciptorSetLayout;

	virtual void CreatePushConstantRange();
	std::vector<vk::PushConstantRange> _PushConstantRange;

	virtual void CreateDescriptorPool(const uint32_t poolSize);
	vk::DescriptorPool _DescriptorPool;
	vk::PipelineLayout _PipelineLayout;

	// Return the bound shaders in a list
	std::vector<vk::PipelineShaderStageCreateInfo> GetShaderInfoList();
	std::unordered_map<vk::ShaderStageFlagBits, Shader> _ShaderMap;

	vk::Pipeline _Pipeline;

	vk::PipelineVertexInputStateCreateInfo _VertexInputInfo;
};