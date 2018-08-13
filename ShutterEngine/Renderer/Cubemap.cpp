#include "Cubemap.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

Cubemap::Cubemap(Device * device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize) :
	Material(device, scene, width, height, poolSize)
{
}

void Cubemap::CreateDescriptorSetLayout()
{
	_LayoutBindings.push_back(vk::DescriptorSetLayoutBinding(
		0,
		vk::DescriptorType::eUniformBuffer,
		1,
		vk::ShaderStageFlagBits::eVertex
	));

	_LayoutBindings.push_back(vk::DescriptorSetLayoutBinding(
		1,
		vk::DescriptorType::eUniformBufferDynamic,
		1,
		vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment
	));

	_LayoutBindings.push_back(vk::DescriptorSetLayoutBinding(
		2,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment
	));

	_DesciptorSetLayout = _Device->GetDevice().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, _LayoutBindings.size(), _LayoutBindings.data()));
}

void Cubemap::CreateRasterizationInfo()
{
	_RasterizationInfo = vk::PipelineRasterizationStateCreateInfo(
		{},
		false,
		false,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eFront,
		vk::FrontFace::eCounterClockwise,
		false
	);
}
