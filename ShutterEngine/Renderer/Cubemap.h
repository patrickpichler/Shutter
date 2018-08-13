#pragma once

#include <string>
#include "Shader.h"
#include "Material.h"

class Cubemap: public Material {
public:
	Cubemap(){}
	Cubemap(Device *device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize);
protected:
	void CreateRasterizationInfo() override;
	void CreateDescriptorSetLayout() override;
};