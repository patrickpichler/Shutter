#pragma once

#include <string>
#include "Shader.h"
#include "Material.h"

class Shadow: public Material {
public:
	Shadow(){}
	Shadow(Device *device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize);

protected:
	virtual void CreateMultisampleInfo() override {
		_MultisampleInfo = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, false);;
	}
	void CreateColorBlendInfo() override {
		_ColorBlendAttachement = vk::PipelineColorBlendAttachmentState(
			false
		);
	}
};