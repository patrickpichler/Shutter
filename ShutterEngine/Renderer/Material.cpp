#include "Material.h"
#include "Engine/Mesh.h"
#include "Helpers.h"

Material::Material(const Device &device, const uint16_t width, const uint16_t height, const uint32_t poolSize)
{
	CreateDescriptorSetLayout(device);
	CreateDescriptorPool(device, poolSize);

	PopulateInfo(width, height);

	pushConstantRange[0].offset = 0;
	pushConstantRange[0].size = sizeof(glm::vec3);
	pushConstantRange[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;


	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &_DesciptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 1;
	pipelineLayoutInfo.pPushConstantRanges = pushConstantRange;

	VkResult result = vkCreatePipelineLayout(device.GetLogicalDevice(), &pipelineLayoutInfo, nullptr, &PipelineLayout);
}

void Material::BindShader(const Shader & shader)
{
	if (!ShaderMap.emplace(std::make_pair(shader.GetStage(), shader)).second) {
		throw std::runtime_error("Shader already bound for this stage.");
	}
}

void Material::CreatePipeline(const Device &device, const VkRenderPass &renderPass)
{
	/// TODO: Clean, moved here to keep pointer context
	vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexAttributeDescriptionCount = Vertex::GetAttributeDescriptions().size();
	vertexInputInfo.pVertexAttributeDescriptions = Vertex::GetAttributeDescriptions().data();
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &Vertex::GetBindingDescription();

	auto stages = GetShaderInfoList();

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = ShaderMap.size();
	pipelineInfo.pStages = stages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pDepthStencilState = &depthStencilInfo;
	pipelineInfo.layout = PipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

	vk_expect_success(vkCreateGraphicsPipelines(device.GetLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_Pipeline), "Failed creating pipeline.");
}

void Material::PopulateInfo(const uint32_t width, const uint32_t height)
{
	CreateInputAssemblyInfo();

	CreateViewportInfo(width, height);

	CreateRasterizationInfo();

	CreateMultisamplingInfo();

	CreateDepthStencilInfo();

	CreateColorBlendInfo();
}

void Material::CreateInputAssemblyInfo()
{
	inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
}

void Material::CreateViewportInfo(const uint32_t width, const uint32_t height)
{
	viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = width;
	viewport.height = height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { width, height };

	viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;
}

void Material::CreateRasterizationInfo()
{
	rasterInfo = {};
	rasterInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterInfo.depthClampEnable = VK_FALSE;
	rasterInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterInfo.lineWidth = 1.0f;
	rasterInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterInfo.depthBiasEnable = VK_FALSE;
}

void Material::CreateMultisamplingInfo()
{
	multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
}

void Material::CreateDepthStencilInfo()
{
	depthStencilInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
	depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.stencilTestEnable = VK_FALSE;
}

void Material::CreateColorBlendInfo()
{
	colorBlend = {};
	colorBlend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlend.blendEnable = VK_FALSE;
	colorBlend.blendEnable = VK_TRUE;
	colorBlend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlend.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlend.alphaBlendOp = VK_BLEND_OP_ADD;

	colorBlendInfo = {};
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlend;
}

void Material::CreateDescriptorSetLayout(const Device & device)
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutBinding modelLayoutBinding = {};
	modelLayoutBinding.binding = 1;
	modelLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	modelLayoutBinding.descriptorCount = 1;
	modelLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerBinding = {};
	samplerBinding.binding = 2;
	samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerBinding.descriptorCount = 1;
	samplerBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	//VkDescriptorSetLayoutBinding samplerSpecBinding = {};
	//samplerSpecBinding.binding = 3;
	//samplerSpecBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//samplerSpecBinding.descriptorCount = 1;
	//samplerSpecBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding samplerNormalBinding = {};
	samplerNormalBinding.binding = 3;
	samplerNormalBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerNormalBinding.descriptorCount = 1;
	samplerNormalBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutBinding bindings[] = { uboLayoutBinding, modelLayoutBinding, samplerBinding, samplerNormalBinding };

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = 4;
	layoutInfo.pBindings = bindings;

	vk_expect_success(vkCreateDescriptorSetLayout(device.GetLogicalDevice(), &layoutInfo, nullptr, &_DesciptorSetLayout), "Failed to create the descriptor set.");
}

void Material::CreateDescriptorPool(const Device & device, const uint32_t poolSize)
{
	VkDescriptorPoolSize poolSizes[4];
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = poolSize;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSizes[1].descriptorCount = poolSize;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[2].descriptorCount = poolSize;
	//poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	//poolSizes[3].descriptorCount = poolSize;
	poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[3].descriptorCount = poolSize;

	VkDescriptorPoolCreateInfo poolInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = 4;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.maxSets = poolSize;

	vk_expect_success(vkCreateDescriptorPool(device.GetLogicalDevice(), &poolInfo, nullptr, &_DescriptorPool), "Fail to create the desciptor pool.");
}

std::vector<VkPipelineShaderStageCreateInfo> Material::GetShaderInfoList()
{
	std::vector<VkPipelineShaderStageCreateInfo> shaderList;
	for (const auto &shader : ShaderMap) {
		shaderList.push_back(shader.second.GetShaderPipelineInfo());
	}

	return shaderList;
}
