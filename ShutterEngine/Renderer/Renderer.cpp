#include "Renderer.h"
#include <iostream>
#include <array>
#include <limits>
#include <algorithm>
#include <iterator>
#include <glm/glm.hpp>
#include "Helpers.h"

void Renderer::Init(GLFWwindow* window, const uint16_t width, const uint16_t height, Scene *scene)
{
	_Window = window;
	_ScreenSize = { width, height };
	_Scene = scene;


	CreateInstance();
	CreateSurface();
	CreateDevice();
	CreateSwapchain();

	_Scene->CreateDescriptorSets(&_Device, _SwapchainImageViews.size());
	CreateCommandPool();

	CreateDepth();
	CreateResolve();


	CreateRenderPass();
	CreateFramebuffers();


	_Scene->Load("sponza", &_Device, _CommandPool, _RenderPass);

	CreateCommandBuffers();
	CreateSemaphores();
}

void Renderer::Draw()
{
	_Device().waitForFences(_InFlightFences[_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	_Device().resetFences(_InFlightFences[_CurrentFrame]);

	uint32_t imageIndex = _Device().acquireNextImageKHR(_Swapchain, std::numeric_limits<uint64_t>::max(), _ImageAvailableSemaphore[_CurrentFrame], {}).value;

	_Scene->Update(_CurrentFrame);

	BuildCommandBuffers();

	_Device.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue.submit(
		{
			vk::SubmitInfo(
				1,
				&_ImageAvailableSemaphore[_CurrentFrame],
				&vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput),
				1,
				&_CommandBuffers[_CurrentFrame],
				1,
				&_RenderFinishedSemaphore[_CurrentFrame]
			)
		},
		_InFlightFences[_CurrentFrame]
	);
	_Device.GetQueue(E_QUEUE_TYPE::PRESENT).VulkanQueue.presentKHR(vk::PresentInfoKHR(
		1,
		&_RenderFinishedSemaphore[_CurrentFrame],
		1,
		&_Swapchain,
		&imageIndex
	));

	_CurrentFrame = (_CurrentFrame + 1) % 2;
}

void Renderer::Clean()
{
	//DepthImage.Clean(DeviceRef);
	//for (auto &frame : FrameBuffers) {
	//	vkDestroyFramebuffer(DeviceRef.GetLogicalDevice(), frame, nullptr);
	//}
	//vkFreeCommandBuffers(DeviceRef.GetLogicalDevice(), CommandPool, CommandBuffers.size(), CommandBuffers.data());

	//vkDestroyRenderPass(DeviceRef.GetLogicalDevice(), RenderPass, nullptr);
	//for (auto &image : SwapChainImageViews) {
	//	image.Clean(DeviceRef);
	//}
	//vkDestroySwapchainKHR(DeviceRef.GetLogicalDevice(), SwapChain, nullptr);

	//for (auto &semaphore : ImageAvailableSemaphore) {
	//	vkDestroySemaphore(DeviceRef.GetLogicalDevice(), semaphore, nullptr);
	//}
	//for (auto &semaphore : RenderFinishedSemaphore) {
	//	vkDestroySemaphore(DeviceRef.GetLogicalDevice(), semaphore, nullptr);
	//}
	//for (auto &fence : InFlightFences) {
	//	vkDestroyFence(DeviceRef.GetLogicalDevice(), fence, nullptr);
	//}

	//vkDestroyCommandPool(DeviceRef.GetLogicalDevice(), CommandPool, nullptr);

	//VertexShader.Clean(DeviceRef);
	//FragmentShader.Clean(DeviceRef);


	//DeviceRef.Clean();

	//layerManager.Clean(Instance);

	//vkDestroySurfaceKHR(Instance, Surface, nullptr);

	//vkDestroyInstance(Instance, nullptr);
}

void Renderer::WaitIdle()
{
	_Device().waitIdle();
}

void Renderer::ReloadShaders()
{
	//// Reload the basic Material
	//std::vector<Shader> shaders =  _BasicMaterial.GetShaderList();
	//_BasicMaterial.ClearShaders();

	//for (auto &shader : shaders) {
	//	Shader newShader(&_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
	//	shader.Clean();
	//	_BasicMaterial.BindShader(newShader);
	//}
	//_BasicMaterial.ReloadPipeline(_RenderPass, _ScreenSize.width, _ScreenSize.height);

	//// Reload the skybox Material
	//shaders = _SkyboxMaterial.GetShaderList();
	//_SkyboxMaterial.ClearShaders();

	//for (auto &shader : shaders) {
	//	Shader newShader(&_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
	//	shader.Clean();
	//	_SkyboxMaterial.BindShader(newShader);
	//}
	//_SkyboxMaterial.ReloadPipeline(_RenderPass, _ScreenSize.width, _ScreenSize.height);
}

void Renderer::CreateInstance()
{
	vk::ApplicationInfo applicationInfo("Demo", VK_MAKE_VERSION(1, 0, 0), "Shutter", VK_MAKE_VERSION(1, 0, 0), VULKAN_VERSION);


	uint32_t glfwExtensionCount = 0;
	const char **glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	ExtensionRequestInfo extensionInfo = {};
	extensionInfo.RequiredExtensions = {
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME
	};
	extensionInfo.RequiredExtensions.insert(extensionInfo.RequiredExtensions.end(), extensions.begin(), extensions.end());

	LayerRequestInfo layerInfo = {};
	layerInfo.RequiredLayers = {
		"VK_LAYER_LUNARG_standard_validation"
	};

	// Query extension and layer capabilities
	_ExtensionManager.Init(extensionInfo);
	_LayerManager.Init(layerInfo);

	 _Instance = vk::createInstance(vk::InstanceCreateInfo(
		{},
		&applicationInfo,
		_LayerManager.GetEnabledLayers().size(),
		_LayerManager.GetEnabledLayers().data(),
		_ExtensionManager.GetEnabledExtensions().size(),
		_ExtensionManager.GetEnabledExtensions().data()
	));

	 _LayerManager.AttachDebugCallback(_Instance);
}

void Renderer::CreateDevice() {
	DeviceRequestInfo deviceRequestInfo = {};
	deviceRequestInfo.RequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	deviceRequestInfo.SupportPresentation = true;
	deviceRequestInfo.SupportGraphics = true;

	_Device = Device::GetDevice(_Instance, deviceRequestInfo, _Surface);
	_Device.Init(deviceRequestInfo, _Surface);
}

void Renderer::CreateSurface()
{
	_Surface = _Instance.createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR({}, GetModuleHandle(nullptr), glfwGetWin32Window(_Window)));
}

void Renderer::CreateSwapchain()
{
	auto& queueIndexSet = _Device.GetQueueIndexSet();
	std::vector<uint32_t> queueIndexList;
	std::copy(queueIndexSet.begin(), queueIndexSet.end(), std::back_inserter(queueIndexList));

	vk::Format swapchainFormat = vk::Format::eR8G8B8A8Unorm;

	_Swapchain = _Device().createSwapchainKHR(vk::SwapchainCreateInfoKHR(
		{},
		_Surface,
		1,
		swapchainFormat,
		vk::ColorSpaceKHR::eSrgbNonlinear,
		_ScreenSize,
		1,
		vk::ImageUsageFlagBits::eColorAttachment,
		vk::SharingMode::eExclusive,
		queueIndexList.size(),
		queueIndexList.data(),
		vk::SurfaceTransformFlagBitsKHR::eIdentity,
		vk::CompositeAlphaFlagBitsKHR::eOpaque,
		vk::PresentModeKHR::eFifo,
		true,
		{}
	));


	std::vector<vk::Image> swapchainImages;
	swapchainImages = _Device().getSwapchainImagesKHR(_Swapchain);

	_SwapchainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
		_SwapchainImageViews[i].FromVkImage(&_Device, swapchainImages[i], swapchainFormat);
	}
}

void Renderer::CreateRenderPass()
{
	// Color Image
	vk::AttachmentDescription colorAttachement(
		{},
		vk::Format::eR8G8B8A8Unorm,
		vk::SampleCountFlagBits::e4,
		vk::AttachmentLoadOp::eClear,
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

	// Depth Image
	vk::AttachmentDescription depthAttachement(
		{},
		_DepthImage.GetFormat(),
		vk::SampleCountFlagBits::e4,
		vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eDontCare,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	vk::AttachmentReference depthAttachementReference(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal
	);

	// Multisample Resolved Image
	vk::AttachmentDescription resolveAttachement(
		{},
		vk::Format::eR8G8B8A8Unorm,
		vk::SampleCountFlagBits::e1,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eStore,
		vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::ePresentSrcKHR
	);

	vk::AttachmentReference resolveAttachementReference(
		2,
		vk::ImageLayout::eColorAttachmentOptimal
	);

	vk::SubpassDescription subpass(
		{},
		vk::PipelineBindPoint::eGraphics,
		0,
		{},
		1,
		&colorAttachementReference,
		&resolveAttachementReference,
		&depthAttachementReference
	);

	std::array<vk::AttachmentDescription, 3> attachements{
		colorAttachement,
		depthAttachement,
		resolveAttachement
	};

	
	_RenderPass = _Device().createRenderPass(vk::RenderPassCreateInfo(
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
			vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite
		)
	));
}

void Renderer::CreateFramebuffers()
{
	_Framebuffers.resize(_SwapchainImageViews.size());

	for (size_t i = 0; i < _SwapchainImageViews.size(); ++i) {
		std::array<vk::ImageView,3> attachments = {
			_ImageResolve[i].GetImageView(),
			_DepthImage.GetImageView(),
			_SwapchainImageViews[i].GetImageView()
		};
		
		_Framebuffers[i] = _Device().createFramebuffer(vk::FramebufferCreateInfo(
			{},
			_RenderPass,
			attachments.size(),
			attachments.data(),
			_ScreenSize.width,
			_ScreenSize.height,
			1
		));
	}
}

void Renderer::CreateCommandPool()
{
	_CommandPool = _Device().createCommandPool(vk::CommandPoolCreateInfo(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, _Device.GetQueue(E_QUEUE_TYPE::GRAPHICS).Index));
}

void Renderer::CreateDepth()
{
	_DepthImage = Image(
		&_Device,
		VkExtent3D{ _ScreenSize.width, _ScreenSize.height, 1 },
		1,
		vk::Format::eD32Sfloat,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		false,
		vk::SampleCountFlagBits::e4
	);
}

void Renderer::CreateResolve()
{
	vk::Format colorFormat = vk::Format::eR8G8B8A8Unorm;

	_ImageResolve.resize(_SwapchainImageViews.size());

	for (size_t i = 0; i < _SwapchainImageViews.size(); i++) {
		_ImageResolve.at(i) = Image(
			&_Device,
			VkExtent3D{ _ScreenSize.width, _ScreenSize.height, 1 },
			1,
			colorFormat,
			vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
			false,
			vk::SampleCountFlagBits::e4
		);
		_ImageResolve.at(i).TransitionLayout(_CommandPool, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);
	}
}

void Renderer::CreateCommandBuffers()
{
	_CommandBuffers = _Device().allocateCommandBuffers(vk::CommandBufferAllocateInfo(_CommandPool, vk::CommandBufferLevel::ePrimary, _Framebuffers.size()));
}

void Renderer::BuildCommandBuffers()
{	
	_CommandBuffers[_CurrentFrame].begin({ vk::CommandBufferUsageFlagBits::eSimultaneousUse });

	std::array<vk::ClearValue, 2> clearValues;
	clearValues[0] = vk::ClearColorValue(std::array<float, 4>{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1] = vk::ClearDepthStencilValue(1.0f, 0);

	_CommandBuffers[_CurrentFrame].beginRenderPass(
		vk::RenderPassBeginInfo(
			_RenderPass,
			_Framebuffers[_CurrentFrame],
			{ {0, 0}, _ScreenSize },
			clearValues.size(),
			clearValues.data()
		),
		vk::SubpassContents::eInline
	);

	_CommandBuffers[_CurrentFrame].bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_Scene->_Materials.at("basic").GetPipelineLayout(),
		0,
		{
			_Scene->GetDescriptorSet(_CurrentFrame)
		},
		{}
	);

	for (const auto &mat : _Scene->_Materials) {
		_CommandBuffers[_CurrentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, mat.second.GetPipeline());


		for (const auto &object : _Scene->_Objects[mat.first]) {
			_CommandBuffers[_CurrentFrame].bindVertexBuffers(0, { object._Mesh._VertexBuffer.GetBuffer() }, { 0 });

			_CommandBuffers[_CurrentFrame].bindDescriptorSets(
				vk::PipelineBindPoint::eGraphics,
				mat.second.GetPipelineLayout(),
				0,
				{
					_Scene->GetDescriptorSet(_CurrentFrame),
					object.GetDescriptorSet(_CurrentFrame)
				},
				{ object._DynamicIndex * static_cast<uint32_t>(Object::dynamicAlignement) }
			);

			_CommandBuffers[_CurrentFrame].draw(object._Mesh._Vertices.size(), 1, 0, 0);
		}
	}

	_CommandBuffers[_CurrentFrame].endRenderPass();
	_CommandBuffers[_CurrentFrame].end();
}

void Renderer::CreateSemaphores()
{
	_ImageAvailableSemaphore.resize(2);
	_RenderFinishedSemaphore.resize(2);
	_InFlightFences.resize(2);

	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint8_t i = 0; i < 2; ++i)
	{
		_ImageAvailableSemaphore[i] = _Device().createSemaphore({});
		_RenderFinishedSemaphore[i] = _Device().createSemaphore({});
		_InFlightFences[i] = _Device().createFence({ vk::FenceCreateFlagBits::eSignaled });
	}
}