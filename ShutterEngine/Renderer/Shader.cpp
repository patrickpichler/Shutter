#include "Shader.h"
#include <fstream>

Shader::Shader(
	const std::string &shaderName,
	const std::string &filename,
	const VkShaderStageFlagBits stage,
	const VkDevice &device,
	const std::string &entrypoint
){
	ShaderName = shaderName;
	EntryPoint = entrypoint;
	ShaderStage = stage;
	LoadFile(filename);
	CreateShaderModule(device);
}

const VkShaderModule& Shader::GetShaderModule() const
{
	return ShaderModule;
}

void Shader::Clean(const Device &device)
{
	vkDestroyShaderModule(device.GetLogicalDevice(), ShaderModule, nullptr);
}

VkPipelineShaderStageCreateInfo Shader::GetShaderPipelineInfo() const
{
	VkPipelineShaderStageCreateInfo shaderInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	shaderInfo.stage = ShaderStage;
	shaderInfo.module = GetShaderModule();
	shaderInfo.pName = EntryPoint.c_str();

	return shaderInfo;
}

void Shader::LoadFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("File " + filename + " not found.");
	}

	size_t fileSize = (size_t)file.tellg();
	ShaderCode.resize(fileSize);
	file.seekg(0);
	file.read(ShaderCode.data(), fileSize);
	file.close();
}

void Shader::CreateShaderModule(const VkDevice &device)
{
	VkShaderModuleCreateInfo shaderModuleInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	shaderModuleInfo.codeSize = ShaderCode.size();
	shaderModuleInfo.pCode = reinterpret_cast<const uint32_t*>(ShaderCode.data());

	if (vkCreateShaderModule(device, &shaderModuleInfo, nullptr, &ShaderModule) != VK_SUCCESS) {
		throw std::runtime_error("Failure to create shader module for " + ShaderName + ".");
	}
}
