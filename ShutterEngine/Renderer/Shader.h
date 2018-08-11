#pragma once
#include <string>
#include <vector>
#include "DeviceHandler.h"

#include <vulkan/vulkan.h>

class Shader {
public:
	Shader() {};

	Shader(
		const std::string &shaderName,
		const std::string &filename,
		const VkShaderStageFlagBits stage,
		const VkDevice &device,
		const std::string &entrypoint = "main"
	);

	const VkShaderModule& GetShaderModule() const;

	void Clean(const Device &device);

	VkPipelineShaderStageCreateInfo GetShaderPipelineInfo() const;
	
	VkShaderStageFlagBits GetStage() const {
		return ShaderStage;
	}


private:
	void LoadFile(const std::string &filename);
	void CreateShaderModule(const VkDevice &device);

private:
	std::string ShaderName;
	std::vector<char> ShaderCode;
	std::string EntryPoint;
	VkShaderStageFlagBits ShaderStage;

	VkShaderModule ShaderModule;
};