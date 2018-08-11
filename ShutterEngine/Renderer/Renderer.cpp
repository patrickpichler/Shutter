#include "Renderer.h"
#include <iostream>
#include <array>
#include <limits>
#include <algorithm>
#include <iterator>
#include <glm/glm.hpp>
#include "Helpers.h"

void Renderer::Init(GLFWwindow* window, const uint16_t width, const uint16_t height, std::shared_ptr<Scene> scene)
{
	ScreenSize = { width, height };
	_Scene = std::move(scene);


	CreateInstance();
	CreateSurface(window);
	CreateDevice();
	CreateSwapChain();

	_Scene->CreateDescriptorSets(&DeviceRef, SwapChainImageViews.size());

	PrepareDynamic();
	CreateCommandPool();

	CreateDepth();
	CreateResolve();


	CreateRenderPass();
	CreateFrameBuffers();


	// Load the cubemap
	
	cbt.Init(DeviceRef, {
		"shaders/cubemap/posx.jpg",
		"shaders/cubemap/negx.jpg",
		"shaders/cubemap/posy.jpg",
		"shaders/cubemap/negy.jpg",
		"shaders/cubemap/posz.jpg",
		"shaders/cubemap/negz.jpg"
	});
	cbt.TransferBufferToImage(DeviceRef, CommandPool);

	Shader vertexSky("Vertex1", "shaders/skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT, DeviceRef.GetLogicalDevice());
	Shader fragmentSky("Fragment1", "shaders/skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, DeviceRef.GetLogicalDevice());

	_SkyboxMaterial = std::make_shared<Cubemap>(
		DeviceRef,
		ScreenSize.width,
		ScreenSize.height,
		static_cast<uint32_t>(SwapChainImageViews.size())
	);

	_SkyboxMaterial->BindShader(vertexSky);
	_SkyboxMaterial->BindShader(fragmentSky);

	// Init the materials

	Shader vertex("Vertex1", "shaders/vert.spv", VK_SHADER_STAGE_VERTEX_BIT, DeviceRef.GetLogicalDevice());
	Shader fragment("Fragment1", "shaders/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT, DeviceRef.GetLogicalDevice());

	_BasicMaterial = std::make_shared<Material>(
		DeviceRef,
		ScreenSize.width,
		ScreenSize.height,
		static_cast<uint32_t>(SwapChainImageViews.size() * 1024)
	);

	_BasicMaterial->BindShader(vertex);
	_BasicMaterial->BindShader(fragment);

	// Load the textures
	//_AppleTexture.Init(DeviceRef, "shaders/apple_Albedo.tga.png");
	//_AppleSpecular.Init(DeviceRef, "shaders/apple_spec.tga.png");
	//_AppleNormal.Init(DeviceRef, "shaders/apple_normals.tga.png");

	// Load the meshes
	//_SponzaMesh.Load(DeviceRef, "shaders/Sponza/sponza.obj");
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "shaders/cube2.obj");
		_Cube.Load(DeviceRef, shapes.front(), attrib);
	}

	_Skybox = Object(DeviceRef, _Cube, std::reinterpret_pointer_cast<Material>(_SkyboxMaterial), SwapChainImageViews.size());
	_Skybox.AddTexture(2, cbt);
	_Skybox.CreateDescriptorSet(DeviceRef);

	LoadObj("shaders/Sponza/sponza.obj");

	// Create the objects
	//_Apple = Object(DeviceRef, _SponzaMesh, _BasicMaterial, static_cast<uint32_t>(SwapChainImageViews.size()));
	//_Apple.AddTexture(2, _AppleTexture);
	//_Apple.AddTexture(3, _AppleSpecular);
	//_Apple.AddTexture(4, _AppleNormal);
	//_Apple.CreateDescriptorSet(DeviceRef);


	_BasicMaterial->CreatePipeline(DeviceRef, RenderPass);
	_SkyboxMaterial->CreatePipeline(DeviceRef, RenderPass);
	//_AppleTexture.TransferBufferToImage(DeviceRef, CommandPool);
	//_AppleSpecular.TransferBufferToImage(DeviceRef, CommandPool);
	//_AppleNormal.TransferBufferToImage(DeviceRef, CommandPool);

	CreateCommandBuffers();
	CreateSemaphores();
}

void Renderer::Draw()
{
	vkWaitForFences(DeviceRef.GetLogicalDevice(), 1, &InFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	vkResetFences(DeviceRef.GetLogicalDevice(), 1, &InFlightFences[currentFrame]);
	uint32_t imageIndex;
	vkAcquireNextImageKHR(DeviceRef.GetLogicalDevice(), SwapChain, std::numeric_limits<uint64_t>::max(), ImageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);

	_Scene->Update(currentFrame);

	BuildCommandBuffers();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { ImageAvailableSemaphore[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &CommandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { RenderFinishedSemaphore[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VkResult result = vkQueueSubmit(DeviceRef.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue, 1, &submitInfo, InFlightFences[currentFrame]);

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { SwapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	vkQueuePresentKHR(DeviceRef.GetQueue(E_QUEUE_TYPE::PRESENT).VulkanQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % 2;
}

void Renderer::Clean()
{
	DepthImage.Clean(DeviceRef);
	for (auto &frame : FrameBuffers) {
		vkDestroyFramebuffer(DeviceRef.GetLogicalDevice(), frame, nullptr);
	}
	vkFreeCommandBuffers(DeviceRef.GetLogicalDevice(), CommandPool, CommandBuffers.size(), CommandBuffers.data());

	vkDestroyRenderPass(DeviceRef.GetLogicalDevice(), RenderPass, nullptr);
	for (auto &image : SwapChainImageViews) {
		image.Clean(DeviceRef);
	}
	vkDestroySwapchainKHR(DeviceRef.GetLogicalDevice(), SwapChain, nullptr);

	for (auto &semaphore : ImageAvailableSemaphore) {
		vkDestroySemaphore(DeviceRef.GetLogicalDevice(), semaphore, nullptr);
	}
	for (auto &semaphore : RenderFinishedSemaphore) {
		vkDestroySemaphore(DeviceRef.GetLogicalDevice(), semaphore, nullptr);
	}
	for (auto &fence : InFlightFences) {
		vkDestroyFence(DeviceRef.GetLogicalDevice(), fence, nullptr);
	}

	vkDestroyCommandPool(DeviceRef.GetLogicalDevice(), CommandPool, nullptr);

	VertexShader.Clean(DeviceRef);
	FragmentShader.Clean(DeviceRef);


	DeviceRef.Clean();

	layerManager.Clean(Instance);

	vkDestroySurfaceKHR(Instance, Surface, nullptr);

	vkDestroyInstance(Instance, nullptr);
}

void Renderer::CreateInstance()
{
	VkApplicationInfo applicationInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	applicationInfo.pApplicationName = "Project0";
	applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.apiVersion = VULKAN_VERSION;
	applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	applicationInfo.pEngineName = "Pemberley";

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
	extensionManager.Init(extensionInfo);
	layerManager.Init(layerInfo);


	VkInstanceCreateInfo instanceInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledExtensionCount = extensionManager.GetEnabledExtensions().size();
	instanceInfo.ppEnabledExtensionNames = extensionManager.GetEnabledExtensions().data();
	instanceInfo.enabledLayerCount = layerManager.GetEnabledLayers().size();
	instanceInfo.ppEnabledLayerNames = layerManager.GetEnabledLayers().data();

	vk_expect_success(vkCreateInstance(&instanceInfo, nullptr, &Instance), "Unable to create the Vulkan instance.");
	vk_expect_success(layerManager.AttachDebugCallback(Instance), "Unable to attach the debug layer callback");
}

void Renderer::CreateDevice() {
	DeviceRequestInfo deviceRequestInfo = {};
	deviceRequestInfo.RequiredExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	deviceRequestInfo.SupportPresentation = true;
	deviceRequestInfo.SupportGraphics = true;

	DeviceRef = DeviceHandler::GetDevice(Instance, deviceRequestInfo, Surface);
	DeviceRef.Init(deviceRequestInfo, Surface);
}

void Renderer::CreateSurface(GLFWwindow* window)
{
	VkWin32SurfaceCreateInfoKHR surfaceInfo = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	surfaceInfo.hwnd = glfwGetWin32Window(window);
	surfaceInfo.hinstance = GetModuleHandle(nullptr);

	auto CreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(Instance, "vkCreateWin32SurfaceKHR");

	vk_expect_success(CreateWin32SurfaceKHR(Instance, &surfaceInfo, nullptr, &Surface), "Fail to create display surface.");
}

void Renderer::PrepareDynamic()
{
	uint32_t minAlignement = DeviceRef.GetProperties().limits.minUniformBufferOffsetAlignment;
	Object::dynamicAlignement = sizeof(glm::mat4);

	if (minAlignement > 0) {
		Object::dynamicAlignement = (Object::dynamicAlignement + minAlignement - 1) & ~(minAlignement - 1);
	}

	uint32_t bufferSize = 2 * Object::dynamicAlignement;
	Object::uboDynamic.model = (glm::mat4*)_aligned_malloc(bufferSize, Object::dynamicAlignement);

	Object::DynamicBuffer = Buffer(DeviceRef, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, bufferSize);
	//dynamicBuffer.Copy(DeviceRef, uboDynamic.model, bufferSize);

	glm::mat4* modelPtr;
	for (uint32_t j = 0; j < 2; j++)
	{
		modelPtr = (glm::mat4*)(((uint64_t)Object::uboDynamic.model + (j * Object::dynamicAlignement)));

		_Apple._Position = glm::vec3(0.0 + 3.0*j, 0.0, 0.0);
		_Apple._Rotation = glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0.0f);
		_Apple._Scale = glm::vec3(0.01f);
		*modelPtr = _Apple.GetModelMatrix();
	}
	modelPtr = (glm::mat4*)(((uint64_t)Object::uboDynamic.model + (1 * Object::dynamicAlignement)));

	_Apple._Position = glm::vec3(0.0, 0.0, 0.0);
	_Apple._Rotation = glm::vec3(glm::radians(90.0f), glm::radians(90.0f), 0.0f);
	_Apple._Scale = glm::vec3(5.0f);
	*modelPtr = _Apple.GetModelMatrix();

	Object::DynamicBuffer.Copy(DeviceRef, Object::uboDynamic.model, bufferSize);
}

void Renderer::CreateSwapChain()
{
	auto& queueIndexSet = DeviceRef.GetQueueIndexSet();
	std::vector<uint32_t> queueIndexList;
	std::copy(queueIndexSet.begin(), queueIndexSet.end(), std::back_inserter(queueIndexList));

	VkFormat swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;

	VkSwapchainCreateInfoKHR swapChainInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	swapChainInfo.surface = Surface;
	swapChainInfo.minImageCount = 1;
	swapChainInfo.imageFormat = swapchainFormat;
	swapChainInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	swapChainInfo.imageExtent = ScreenSize;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.queueFamilyIndexCount = queueIndexList.size();
	swapChainInfo.pQueueFamilyIndices = queueIndexList.data();
	swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	swapChainInfo.clipped = true;
	swapChainInfo.oldSwapchain = VK_NULL_HANDLE;

	vk_expect_success(vkCreateSwapchainKHR(DeviceRef.GetLogicalDevice(), &swapChainInfo, nullptr, &SwapChain), "Fail to create the swapchain.");

	std::vector<VkImage> swapchainImages;
	uint32_t imageCount = 0;
	vkGetSwapchainImagesKHR(DeviceRef.GetLogicalDevice(), SwapChain, &imageCount, nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(DeviceRef.GetLogicalDevice(), SwapChain, &imageCount, swapchainImages.data());

	SwapChainImageViews.resize(swapchainImages.size());

	for (uint32_t i = 0; i < swapchainImages.size(); ++i) {
		SwapChainImageViews[i].Init(DeviceRef, swapchainImages[i], swapchainFormat);
	}
}

void Renderer::CreateRenderPass()
{
	VkAttachmentDescription colorAttachement = {};
	colorAttachement.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachement.samples = VK_SAMPLE_COUNT_4_BIT;
	colorAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachement.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachement.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachementRef = {};
	colorAttachementRef.attachment = 0;
	colorAttachementRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachement = {};
	depthAttachement.format = DepthImage.GetFormat();
	depthAttachement.samples = VK_SAMPLE_COUNT_4_BIT;
	depthAttachement.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachement.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachement.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachement.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachement.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachement.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachementRef = {};
	depthAttachementRef.attachment = 1;
	depthAttachementRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachmentResolve = {};
	colorAttachmentResolve.format = VK_FORMAT_B8G8R8A8_UNORM;
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentResolveRef = {};
	colorAttachmentResolveRef.attachment = 2;
	colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;


	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachementRef;
	subpass.pDepthStencilAttachment = &depthAttachementRef;
	subpass.pResolveAttachments = &colorAttachmentResolveRef;

	VkAttachmentDescription attachements[] = {
		colorAttachement,
		depthAttachement,
		colorAttachmentResolve
	};

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 3;
	renderPassInfo.pAttachments = attachements;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VkResult result = vkCreateRenderPass(DeviceRef.GetLogicalDevice(), &renderPassInfo, nullptr, &RenderPass);
}

void Renderer::CreateFrameBuffers()
{
	FrameBuffers.resize(SwapChainImageViews.size());

	for (uint32_t i = 0; i < SwapChainImageViews.size(); ++i) {
		VkImageView attachments[] = {
			ImageResolve[i].GetImageView(),
			DepthImage.GetImageView(),
			SwapChainImageViews[i].GetImageView()
		};

		VkFramebufferCreateInfo frameBufferInfo = {};
		frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferInfo.renderPass = RenderPass;
		frameBufferInfo.attachmentCount = 3;
		frameBufferInfo.pAttachments = attachments;
		frameBufferInfo.width = ScreenSize.width;
		frameBufferInfo.height = ScreenSize.height;
		frameBufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(DeviceRef.GetLogicalDevice(), &frameBufferInfo, nullptr, &FrameBuffers[i]);
	}
}

void Renderer::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = DeviceRef.GetQueue(E_QUEUE_TYPE::GRAPHICS).Index;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VkResult result = vkCreateCommandPool(DeviceRef.GetLogicalDevice(), &poolInfo, nullptr, &CommandPool);
}

void Renderer::CreateDepth()
{
	DepthImage.Init(
		DeviceRef,
		VkExtent3D{ ScreenSize.width, ScreenSize.height, 1 },
		1,
		VK_FORMAT_D32_SFLOAT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		false,
		VK_SAMPLE_COUNT_4_BIT
	);
}

void Renderer::CreateResolve()
{
	VkFormat colorFormat = VK_FORMAT_B8G8R8A8_UNORM;

	ImageResolve.resize(SwapChainImageViews.size());

	for (size_t i = 0; i < SwapChainImageViews.size(); i++) {
		ImageResolve.at(i).Init(
			DeviceRef,
			VkExtent3D{ ScreenSize.width, ScreenSize.height, 1 },
			1,
			colorFormat,
			VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			false,
			VK_SAMPLE_COUNT_4_BIT
		);
		ImageResolve.at(i).TransitionLayout(DeviceRef, CommandPool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
	}
}

void Renderer::CreateCommandBuffers()
{
	CommandBuffers.resize(FrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = CommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

	VkResult result = vkAllocateCommandBuffers(DeviceRef.GetLogicalDevice(), &allocInfo, CommandBuffers.data());
}

void Renderer::BuildCommandBuffers()
{
	uint32_t i = currentFrame;
	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

	VkResult result = vkBeginCommandBuffer(CommandBuffers[i], &beginInfo);

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = RenderPass;
	renderPassInfo.framebuffer = FrameBuffers[i];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = ScreenSize;

	VkClearValue clearVal[] = {
		{ 0.0f, 0.0f, 0.0f, 1.0f },
	{ 1.0f, 0 }
	};
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = clearVal;

	vkCmdBeginRenderPass(CommandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vk::CommandBuffer cmdBuffer(CommandBuffers[i]);

	vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _BasicMaterial->GetPipeline());

	for (uint32_t j = 0; j < _SceneObjects.size(); j++)
	{
		VkBuffer vertexBuffers[] = { _SceneMeshes[j].VertexBuffer.GetBuffer() };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);

		// One dynamic offset per dynamic descriptor to offset into the ubo containing all model matrices
		//uint32_t dynamicOffset = j * static_cast<uint32_t>(Object::dynamicAlignement);*
		uint32_t dynamicOffset = 0;

		VkDescriptorSet dset[] = { VkDescriptorSet(_Scene->GetDescriptorSet(i)), _SceneObjects[j].GetDescriptorSet(i) };

		vkCmdBindDescriptorSets(
			CommandBuffers[i],
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			_BasicMaterial->GetPipelineLayout(),
			0,
			2,
			dset,
			1,
			&dynamicOffset
		);

		vkCmdDraw(CommandBuffers[i], _SceneMeshes[j].Vertices.size(), 1, 0, 0);
	}


	// Draw the skybox
	vkCmdBindPipeline(CommandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _SkyboxMaterial->GetPipeline());
	VkBuffer vertexBuffers[] = { _Cube.VertexBuffer.GetBuffer() };
	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(CommandBuffers[i], 0, 1, vertexBuffers, offsets);

	uint32_t dynamicOffset = 1 * static_cast<uint32_t>(Object::dynamicAlignement);
	vkCmdBindDescriptorSets(
		CommandBuffers[i],
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		_SkyboxMaterial->GetPipelineLayout(),
		0,
		1,
		&_Skybox.GetDescriptorSet(i),
		1,
		&dynamicOffset
	);

	vkCmdDraw(CommandBuffers[i], _Cube.Vertices.size(), 1, 0, 0);


	vkCmdEndRenderPass(CommandBuffers[i]);
	vk_expect_success(vkEndCommandBuffer(CommandBuffers[i]), "Unable to end command buffer.");
}

void Renderer::CreateSemaphores()
{
	ImageAvailableSemaphore.resize(2);
	RenderFinishedSemaphore.resize(2);
	InFlightFences.resize(2);

	VkSemaphoreCreateInfo semaphoreInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint8_t i = 0; i < 2; ++i) {
		vk_expect_success(vkCreateSemaphore(DeviceRef.GetLogicalDevice(), &semaphoreInfo, nullptr, &ImageAvailableSemaphore[i]), "Fail to create semaphore.");
		vk_expect_success(vkCreateSemaphore(DeviceRef.GetLogicalDevice(), &semaphoreInfo, nullptr, &RenderFinishedSemaphore[i]), "Fail to create semaphore.");
		vk_expect_success(vkCreateFence(DeviceRef.GetLogicalDevice(), &fenceInfo, nullptr, &InFlightFences[i]), "Fail to create fence.");
	}
}

void Renderer::LoadObj(const std::string &path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), "shaders/Sponza/");

	//for (const auto& shape : shapes) {
	for (size_t i = 0; i<shapes.size(); ++i) {
		Mesh tempMesh;
		tempMesh.Load(DeviceRef, shapes.at(i), attrib);
		_SceneMeshes.push_back(tempMesh);

		Object object(DeviceRef, tempMesh, _BasicMaterial, static_cast<uint32_t>(SwapChainImageViews.size()));
		_SceneObjects.push_back(object);

		// Add the diffuse texture if there are some provided
		if (materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname != "") {
			std::string path = "shaders/Sponza/" + materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname;

			bool found = false;
			for (auto &tex : _SceneTextures) {
				if (tex.GetFilename() == path) {
					_SceneObjects.back().AddTexture(2, tex);
					found = true;
					break;
				}
			}
			if (!found) {
				Texture tempTex;
				tempTex.Init(DeviceRef, path, true);
				_SceneTextures.push_back(tempTex);

				_SceneObjects.back().AddTexture(2, _SceneTextures.back());
				_SceneTextures.back().TransferBufferToImage(DeviceRef, CommandPool);
			}
		}
		else {
			_SceneObjects.back().AddTexture(2, _SceneTextures.front());
		}

		//// Add the bump texture if there are some provided
		//if (materials.at(shapes.at(i).mesh.material_ids.front()).bump_texname != "") {
		//	std::string path = "shaders/Sponza/" + materials.at(shapes.at(i).mesh.material_ids.front()).bump_texname;

		//	bool found = false;
		//	for (auto &tex : _SceneTextures) {
		//		if (tex.GetFilename() == path) {
		//			_SceneObjects.back().AddTexture(3, tex);
		//			found = true;
		//			break;
		//		}
		//	}
		//	if (!found) {
		//		Texture tempTex;
		//		tempTex.Init(DeviceRef, path);
		//		_SceneTextures.push_back(tempTex);

		//		_SceneObjects.back().AddTexture(3, _SceneTextures.back());
		//		_SceneTextures.back().TransferBufferToImage(DeviceRef, CommandPool);
		//	}
		//}
		//else {
		//	_SceneObjects.back().AddTexture(3, _SceneTextures.front());
		//}

		_SceneObjects.back().CreateDescriptorSet(DeviceRef);
	}

}