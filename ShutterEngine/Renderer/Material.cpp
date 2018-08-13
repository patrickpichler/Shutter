#include "Material.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

Material::Material(Device *device, Scene *scene, const uint16_t width, const uint16_t height, const uint32_t poolSize) :
	_Device(device),
	_Scene(scene)
{
	CreateDescriptorSetLayout();
	CreatePushConstantRange();
	CreateDescriptorPool(poolSize);

	PopulateInfo(width, height);

	std::array<vk::DescriptorSetLayout, 2> descriptorSetLayouts = { _Scene->GetDescriptorSetLayout(), _DesciptorSetLayout };

	_PipelineLayout = _Device->GetDevice().createPipelineLayout(
		vk::PipelineLayoutCreateInfo(
			{},
			descriptorSetLayouts.size(), descriptorSetLayouts.data(),
			_PushConstantRange.size(), _PushConstantRange.data()
		)
	);
}

void Material::BindShader(const Shader & shader)
{
	if (!_ShaderMap.emplace(std::make_pair(shader._Stage, shader)).second) {
		throw std::runtime_error("Shader already bound for this stage.");
	}
}

void Material::CreatePipeline(const vk::RenderPass &renderPass)
{
	/// TODO: Clean, moved here to keep pointer context
	_VertexInputInfo = vk::PipelineVertexInputStateCreateInfo(
		{},
		static_cast<uint32_t>(1),
		&Vertex::GetBindingDescription(),
		static_cast<uint32_t>(Vertex::GetAttributeDescriptions().size()),
		Vertex::GetAttributeDescriptions().data()
	);

	auto stages = GetShaderInfoList();

	vk::GraphicsPipelineCreateInfo pipelineInfo(
		{},
		static_cast<uint32_t>(_ShaderMap.size()),
		stages.data(),
		&_VertexInputInfo,
		&_InputAssemblyInfo,
		nullptr,
		&_ViewportInfo,
		&_RasterizationInfo,
		&_MultisampleInfo,
		&_DepthStencilInfo,
		&_ColorBlendInfo,
		nullptr,
		_PipelineLayout,
		renderPass,
		0
	);

	_Pipeline = _Device->GetDevice().createGraphicsPipeline(nullptr, pipelineInfo);
}

void Material::PopulateInfo(const uint32_t width, const uint32_t height)
{
	CreateInputAssemblyInfo();

	CreateViewportInfo(width, height);

	CreateRasterizationInfo();

	CreateMultisampleInfo();

	CreateDepthStencilInfo();

	CreateColorBlendInfo();
}

void Material::CreateInputAssemblyInfo()
{
	_InputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, false);
}

void Material::CreateViewportInfo(const uint32_t width, const uint32_t height)
{
	_Viewport = vk::Viewport(0.0f, 0.0f, width, height, 0.0f, 1.0f);
	_Scissor = vk::Rect2D({ 0, 0 }, { width, height });
	_ViewportInfo = vk::PipelineViewportStateCreateInfo({}, 1, &_Viewport, 1, &_Scissor);
}

void Material::CreateRasterizationInfo()
{
	_RasterizationInfo = vk::PipelineRasterizationStateCreateInfo(
		{},
		false,
		false,
		vk::PolygonMode::eFill,
		vk::CullModeFlagBits::eBack,
		vk::FrontFace::eCounterClockwise,
		false,
		0,
		0,
		0,
		1.0
	);
}

void Material::CreateMultisampleInfo()
{
	_MultisampleInfo = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e4, false);
}

void Material::CreateDepthStencilInfo()
{
	_DepthStencilInfo = vk::PipelineDepthStencilStateCreateInfo(
		{},
		true,
		true,
		vk::CompareOp::eLess,
		false,
		false
	);
}

void Material::CreateColorBlendInfo()
{
	_ColorBlendAttachement = vk::PipelineColorBlendAttachmentState(
		true,
		vk::BlendFactor::eSrcAlpha,
		vk::BlendFactor::eOneMinusSrcAlpha,
		vk::BlendOp::eAdd,
		vk::BlendFactor::eOne,
		vk::BlendFactor::eZero,
		vk::BlendOp::eAdd,
		vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	);

	_ColorBlendInfo = vk::PipelineColorBlendStateCreateInfo(
		{},
		false,
		{},
		1,
		&_ColorBlendAttachement
	);
}

void Material::CreatePushConstantRange()
{
	_PushConstantRange.push_back(vk::PushConstantRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(glm::vec3)));
}

void Material::CreateDescriptorSetLayout()
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

	_LayoutBindings.push_back(vk::DescriptorSetLayoutBinding(
		3,
		vk::DescriptorType::eCombinedImageSampler,
		1,
		vk::ShaderStageFlagBits::eFragment
	));

	_DesciptorSetLayout = _Device->GetDevice().createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, _LayoutBindings.size(), _LayoutBindings.data()));
}

void Material::CreateDescriptorPool(const uint32_t poolSize)
{
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.resize(_LayoutBindings.size());

	for (size_t i = 0; i < _LayoutBindings.size(); ++i) {
		poolSizes[i].type = _LayoutBindings[i].descriptorType;
		poolSizes[i].descriptorCount = poolSize;
	}

	_DescriptorPool = _Device->GetDevice().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, poolSize, poolSizes.size(), poolSizes.data()));
}

std::vector<vk::PipelineShaderStageCreateInfo> Material::GetShaderInfoList()
{
	std::vector<vk::PipelineShaderStageCreateInfo> shaderList;
	for (auto &shader : _ShaderMap) {
		shaderList.push_back(shader.second.GetShaderPipelineInfo());
	}

	return shaderList;
}
