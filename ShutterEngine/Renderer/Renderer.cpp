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
	PrepareDynamic();
	CreateCommandPool();

	CreateDepth();
	CreateResolve();


	CreateRenderPass();
	CreateFramebuffers();


	// Create and prepare the cubemap material
	_SkyboxTexture = CubeTexture(&_Device);
	_SkyboxTexture.Load({
		"shaders/cubemap/posx.jpg",
		"shaders/cubemap/negx.jpg",
		"shaders/cubemap/posy.jpg",
		"shaders/cubemap/negy.jpg",
		"shaders/cubemap/posz.jpg",
		"shaders/cubemap/negz.jpg"
	});
	_SkyboxTexture.TransferBufferToImage(_CommandPool);

	// Create and prepare the skybox material
	Shader vertexSky(&_Device, "CubemapVertex", "shaders/skybox.vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragmentSky(&_Device, "CubemapFragment", "shaders/skybox.frag.spv", vk::ShaderStageFlagBits::eFragment);

	_SkyboxMaterial = Cubemap(
		&_Device,
		_Scene,
		_ScreenSize.width,
		_ScreenSize.height,
		static_cast<uint32_t>(_SwapchainImageViews.size())
	);

	_SkyboxMaterial.BindShader(vertexSky);
	_SkyboxMaterial.BindShader(fragmentSky);

	// Create and prepare the default material
	Shader vertex(&_Device, "Vertex", "shaders/vert.spv", vk::ShaderStageFlagBits::eVertex);
	Shader fragment(&_Device, "Fragment", "shaders/frag.spv", vk::ShaderStageFlagBits::eFragment);

	_BasicMaterial = Material(
		&_Device,
		_Scene,
		_ScreenSize.width,
		_ScreenSize.height,
		static_cast<uint32_t>(_SwapchainImageViews.size() * 1024)
	);

	_BasicMaterial.BindShader(vertex);
	_BasicMaterial.BindShader(fragment);

	// Load the cube object
	{
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, "shaders/cube2.obj");
		_Cube = Mesh(&_Device);
		_Cube.Load(shapes.front(), attrib);

		_Skybox = Object(&_Device, _Cube, &_SkyboxMaterial, _SwapchainImageViews.size());
		_Skybox.AddTexture(1, _SkyboxTexture);
		_Skybox.CreateDescriptorSet();
	}

	//Load the Sponza object
	LoadObj("shaders/Sponza/sponza.obj");

	_BasicMaterial.CreatePipeline(_RenderPass);
	_SkyboxMaterial.CreatePipeline(_RenderPass);

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
	// Reload the basic Material
	std::vector<Shader> shaders =  _BasicMaterial.GetShaderList();
	_BasicMaterial.ClearShaders();

	for (auto &shader : shaders) {
		Shader newShader(&_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
		shader.Clean();
		_BasicMaterial.BindShader(newShader);
	}
	_BasicMaterial.ReloadPipeline(_RenderPass, _ScreenSize.width, _ScreenSize.height);

	// Reload the skybox Material
	shaders = _SkyboxMaterial.GetShaderList();
	_SkyboxMaterial.ClearShaders();

	for (auto &shader : shaders) {
		Shader newShader(&_Device, shader._Name, shader._Filename, shader._Stage, shader._EntryPoint);
		shader.Clean();
		_SkyboxMaterial.BindShader(newShader);
	}
	_SkyboxMaterial.ReloadPipeline(_RenderPass, _ScreenSize.width, _ScreenSize.height);
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

void Renderer::PrepareDynamic()
{
	uint32_t minAlignement = _Device.GetProperties().limits.minUniformBufferOffsetAlignment;
	Object::dynamicAlignement = sizeof(glm::mat4);

	if (minAlignement > 0) {
		Object::dynamicAlignement = (Object::dynamicAlignement + minAlignement - 1) & ~(minAlignement - 1);
	}

	uint32_t bufferSize = 2 * Object::dynamicAlignement;
	Object::uboDynamic.model = (glm::mat4*)_aligned_malloc(bufferSize, Object::dynamicAlignement);

	Object::DynamicBuffer = Buffer(&_Device, vk::BufferUsageFlagBits::eUniformBuffer, bufferSize);

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

	Object::DynamicBuffer.Copy(Object::uboDynamic.model, bufferSize);
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
		_BasicMaterial.GetPipelineLayout(),
		0,
		{
			_Scene->GetDescriptorSet(_CurrentFrame)
		},
		{}
	);


	// Draw Sponza
	_CommandBuffers[_CurrentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, _BasicMaterial.GetPipeline());

	for (uint32_t j = 0; j < _SceneMeshes.size(); j++)
	{
		_CommandBuffers[_CurrentFrame].bindVertexBuffers(0, { _SceneMeshes[j]._VertexBuffer.GetBuffer() }, { 0 });

		_CommandBuffers[_CurrentFrame].bindDescriptorSets(
			vk::PipelineBindPoint::eGraphics,
			_BasicMaterial.GetPipelineLayout(),
			0,
			{
				_Scene->GetDescriptorSet(_CurrentFrame),
				_SceneObjects[j].GetDescriptorSet(_CurrentFrame)
			},
			{ 0 }
		);

		_CommandBuffers[_CurrentFrame].draw(_SceneMeshes[j]._Vertices.size(), 1, 0, 0);
	}


	 //Draw the skybox
	_CommandBuffers[_CurrentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, _SkyboxMaterial.GetPipeline());

	_CommandBuffers[_CurrentFrame].bindVertexBuffers(0, { _Cube._VertexBuffer.GetBuffer() }, { 0 });

	_CommandBuffers[_CurrentFrame].bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		_SkyboxMaterial.GetPipelineLayout(),
		1,
		{
			_Skybox.GetDescriptorSet(_CurrentFrame)
		},
		{ 1 * static_cast<uint32_t>(Object::dynamicAlignement) }
	);

	_CommandBuffers[_CurrentFrame].draw(_Cube._Vertices.size(), 1, 0, 0);

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

void Renderer::LoadObj(const std::string &path)
{
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string err;
	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str(), "shaders/Sponza/");

	for (size_t i = 0; i<shapes.size(); ++i) {
		Mesh tempMesh(&_Device);
		tempMesh.Load(shapes.at(i), attrib);
		_SceneMeshes.push_back(tempMesh);

		_SceneObjects.push_back(Object(&_Device, _SceneMeshes.back(), &_BasicMaterial, _SwapchainImageViews.size()));

		// Add the diffuse texture if there are some provided
		if (materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname != "") {
			std::string path = "shaders/Sponza/" + materials.at(shapes.at(i).mesh.material_ids.front()).ambient_texname;

			bool found = false;
			for (auto &tex : _SceneTextures) {
				if (tex.GetFilename() == path) {
					_SceneObjects.back().AddTexture(1, tex);
					found = true;
					break;
				}
			}
			if (!found) {
				Texture tempTex(&_Device);
				tempTex.Load(path, true);
				_SceneTextures.push_back(tempTex);

				_SceneObjects.back().AddTexture(1, _SceneTextures.back());
				_SceneTextures.back().TransferBufferToImage(_CommandPool);
			}
		}
		else {
			_SceneObjects.back().AddTexture(1, _SceneTextures.front());
		}

		_SceneObjects.back().CreateDescriptorSet();
	}

}