#pragma once
#include <string>
#include <vector>
#include "DeviceHandler.h"

#include <vulkan/vulkan.hpp>

class Shader {
public:
	Shader(){}
	explicit Shader(
		Device *device,
		const std::string &shaderName,
		const std::string &filename,
		const vk::ShaderStageFlagBits stage,
		const std::string &entrypoint = "main"
	);

	void Clean();

	const vk::ShaderModule& GetShaderModule() const {
		return _Module;
	}

	vk::PipelineShaderStageCreateInfo GetShaderPipelineInfo() const;


private:
	void LoadFile(const std::string &filename);
	void CreateShaderModule();

public:
	vk::ShaderStageFlagBits _Stage;


	std::string _Name;
	std::string _Filename;
	std::string _EntryPoint;

private:
	Device *_Device;
	std::vector<char> _Code;

	vk::ShaderModule _Module;
};