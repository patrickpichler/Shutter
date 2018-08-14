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
	~Shader() {
		Clean();
	}

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

private:
	Device *_Device;

	std::string _Name;
	std::vector<char> _Code;
	std::string _EntryPoint;

	vk::ShaderModule _Module;
};