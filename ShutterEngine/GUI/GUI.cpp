#include "GUI.h"
#include "Renderer/Helpers.h"
#include "Engine/Object.h"

void GUI::Init(Device *device, GLFWwindow *window, const vk::SurfaceKHR & surface, const vk::Extent2D & screenSize, const vk::Instance & instance, vk::SwapchainKHR &swapchain, const vk::CommandPool &cmdPool)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForVulkan(window, true);

	_Device = device;

	CreateDescriptorPool();
	CreateRenderPass();
	CreateCommandBuffers(cmdPool);

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = VkInstance(instance);
	init_info.PhysicalDevice = VkPhysicalDevice(_Device->GetPhysicalDevice());
	init_info.Device = VkDevice(_Device->GetDevice());
	init_info.QueueFamily = _Device->GetQueue(E_QUEUE_TYPE::GRAPHICS).Index;
	init_info.Queue = VkQueue(_Device->GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue);
	init_info.DescriptorPool = VkDescriptorPool(_DescriptorPool);
	ImGui_ImplVulkan_Init(&init_info, VkRenderPass(_RenderPass));

	// Upload Fonts
	{
		// Use any command queue
		vk::CommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(*_Device, cmdPool);
		VkCommandBuffer command_buffer = VkCommandBuffer(cmdBuffer);

		ImGui_ImplVulkan_CreateFontsTexture(command_buffer);

		EndSingleUseCommandBuffer(cmdBuffer, *_Device, cmdPool);
		command_buffer = VK_NULL_HANDLE;

		ImGui_ImplVulkan_InvalidateFontUploadObjects();
	}

	controls._SceneTree = &tree;

}

void GUI::Render(const size_t frameId, const std::vector<vk::Framebuffer> &fb, std::vector<vk::Fence> &fence, std::vector<vk::Fence> &frameFence, std::vector<vk::Semaphore> &semaphore, Scene *scene)
{

	_Device->GetDevice().waitForFences(fence[frameId], VK_TRUE, std::numeric_limits<uint64_t>::max());
	_Device->GetDevice().resetFences(fence[frameId]);

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	perf.Draw();
	tree.Draw();
	controls.Draw();


	ImGui::Render();

	std::array<vk::ClearValue, 1> clearValues;
	clearValues[0] = vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f });
	
	_CommandBuffers[frameId].begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });
	_CommandBuffers[frameId].beginRenderPass(
		vk::RenderPassBeginInfo(
			_RenderPass,
			fb[frameId],
			{ {0,0}, {1024, 768} },
			1,
			clearValues.data()
		),
		vk::SubpassContents::eInline
	);
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VkCommandBuffer(_CommandBuffers[frameId]));
	_CommandBuffers[frameId].endRenderPass();
	_CommandBuffers[frameId].end();

	_Device->GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue.submit(
		{
			vk::SubmitInfo(
				0,
				nullptr,
				&vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput),
				1,
				&_CommandBuffers[frameId],
				1,
				&semaphore[frameId]
			)
		},
		frameFence[frameId]
	);


}

void GUI::CreateRenderPass()
{
	vk::AttachmentDescription colorAttachement(
		{},
		vk::Format::eR8G8B8A8Unorm,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eLoad,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);

	vk::AttachmentReference colorAttachementReference(
		0,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::SubpassDescription subpass(
		{},
		vk::PipelineBindPoint::eGraphics,
		0,
		{},
		1,
		&colorAttachementReference
	);

	std::array<vk::AttachmentDescription, 1> attachements{
		colorAttachement
	};


	_RenderPass = _Device->GetDevice().createRenderPass(vk::RenderPassCreateInfo(
		{},
		attachements.size(),
		attachements.data(),
		1,
		&subpass,
		1,
		&vk::SubpassDependency(
			VK_SUBPASS_EXTERNAL,
			0,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			vk::PipelineStageFlagBits::eColorAttachmentOutput,
			{},
			vk::AccessFlagBits::eColorAttachmentWrite
		)
	));
}

void GUI::CreateDescriptorPool()
{
	std::array<vk::DescriptorPoolSize, 11> poolSize;
	poolSize[0] = { vk::DescriptorType::eSampler, 1000 };
	poolSize[1] = { vk::DescriptorType::eCombinedImageSampler, 1000 };
	poolSize[2] = { vk::DescriptorType::eSampledImage, 1000 };
	poolSize[3] = { vk::DescriptorType::eStorageImage, 1000 };
	poolSize[4] = { vk::DescriptorType::eUniformTexelBuffer, 1000 };
	poolSize[5] = { vk::DescriptorType::eStorageTexelBuffer, 1000 };
	poolSize[6] = { vk::DescriptorType::eUniformBuffer, 1000 };
	poolSize[7] = { vk::DescriptorType::eStorageBuffer, 1000 };
	poolSize[8] = { vk::DescriptorType::eUniformBufferDynamic, 1000 };
	poolSize[9] = { vk::DescriptorType::eStorageBufferDynamic, 1000 };
	poolSize[10] = { vk::DescriptorType::eInputAttachment, 1000 };

	_DescriptorPool = _Device->GetDevice().createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 2, poolSize.size(), poolSize.data()));
}

void GUI::CreateCommandBuffers(const vk::CommandPool &_CommandPool)
{
	_CommandBuffers = _Device->GetDevice().allocateCommandBuffers(vk::CommandBufferAllocateInfo(_CommandPool, vk::CommandBufferLevel::ePrimary, 2));
}
