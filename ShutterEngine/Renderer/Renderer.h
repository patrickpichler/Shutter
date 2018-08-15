#pragma once
#include <vector>
#include <memory>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include "Shader.h"
#include "Extensions.h"
#include "Layers.h"
#include "DeviceHandler.h"
#include "Engine/Mesh.h"

#include "Engine/Object.h"

#include "Material.h"

#define VULKAN_VERSION VK_API_VERSION_1_0

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include "Image.h"

#include "Engine/Texture.h"
#include "Engine/Camera.h"
#include "Engine/Scene.h"

#include "Renderer/Cubemap.h"

#include "Engine/CubeTexture.h"

#include "vulkan/vulkan.hpp"

class Renderer {
public:
	Renderer(){
	}

	void Init(GLFWwindow* window, const uint16_t width, const uint16_t height, Scene *scene);
	void Draw();
	void Clean();

	void WaitIdle();

	void ReloadShaders();

private:
	void CreateInstance();
	void CreateDevice();
	void CreateSurface();
	void PrepareDynamic();
	void CreateSwapchain();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateDepth();
	void CreateResolve();
	void CreateCommandBuffers();
	void BuildCommandBuffers();
	void CreateSemaphores();

	void LoadObj(const std::string &path);
private:
	// Screen/window related
	GLFWwindow *_Window;
	vk::Extent2D _ScreenSize;

	// Rendering related
	Extension _ExtensionManager;
	Layer _LayerManager;

	Device _Device;
	vk::Instance _Instance;

	vk::SurfaceKHR _Surface;

	vk::SwapchainKHR _Swapchain;
	std::vector<Image> _SwapchainImageViews;
	std::vector<Image> _ImageResolve;
	Image _DepthImage;

	std::vector<vk::Framebuffer> _Framebuffers;

	vk::RenderPass _RenderPass;

	vk::CommandPool _CommandPool;
	std::vector<vk::CommandBuffer> _CommandBuffers;

	size_t _CurrentFrame = 0;

	// Sync related
	std::vector<vk::Semaphore> _ImageAvailableSemaphore;
	std::vector<vk::Semaphore> _RenderFinishedSemaphore;
	std::vector<vk::Fence> _InFlightFences;

	// Scene/Objects related
	Scene *_Scene;

	Mesh _Cube;
	std::vector<Mesh> _SceneMeshes;

	CubeTexture _SkyboxTexture;
	std::vector<Texture> _SceneTextures;

	Object _Skybox;
	Object _Apple;
	std::vector<Object> _SceneObjects;

	Material _BasicMaterial;
	Cubemap _SkyboxMaterial;
};