#include "Cubemap.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

Cubemap::Cubemap(Device * device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize) :
	Material(device, scene, width, height, poolSize)
{
	// Investigate impact
	PopulateInfo(width, height);
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
		false,
		0,
		0,
		0,
		1.0
	);
}
