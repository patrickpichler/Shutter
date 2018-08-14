#include "Shader.h"
#include <fstream>

Shader::Shader(
	Device *device,
	const std::string &shaderName,
	const std::string &filename,
	const vk::ShaderStageFlagBits stage,
	const std::string &entrypoint
):
	_Device(device),
	_Stage(stage),
	_Name(shaderName),
	_EntryPoint(entrypoint)
{
	LoadFile(filename);
	CreateShaderModule();
}

void Shader::Clean()
{
	//_Device->GetDevice().destroyShaderModule(_Module);
}

vk::PipelineShaderStageCreateInfo Shader::GetShaderPipelineInfo() const
{
	return vk::PipelineShaderStageCreateInfo({}, _Stage, _Module, _EntryPoint.c_str());
}

void Shader::LoadFile(const std::string &filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("File " + filename + " not found.");
	}

	size_t fileSize = (size_t)file.tellg();
	_Code.resize(fileSize);
	file.seekg(0);
	file.read(_Code.data(), fileSize);
	file.close();
}

void Shader::CreateShaderModule()
{
	_Module = _Device->GetDevice().createShaderModule(vk::ShaderModuleCreateInfo({}, _Code.size(), reinterpret_cast<const uint32_t*>(_Code.data())));
}
