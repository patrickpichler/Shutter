#pragma once
#include <vector>
#include <memory>
#define NOMINMAX
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
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

class Renderer {
public:
	Renderer(){
	}

	void Init(GLFWwindow* window, const uint16_t width, const uint16_t height, std::shared_ptr<Scene> scene);
	void Draw();
	void Clean();

	const Device &GetDevice() const {
		return DeviceRef;
	}
private:
	void CreateInstance();
	void CreateDevice();
	void CreateSurface(GLFWwindow* window);
	void PrepareDynamic();
	void CreateSwapChain();
	void CreateRenderPass();
	void CreateFrameBuffers();
	void CreateCommandPool();
	void CreateDepth();
	void CreateResolve();
	void CreateCommandBuffers();
	void BuildCommandBuffers();
	void CreateSemaphores();

	void LoadObj(const std::string &path);
private:

	Extension extensionManager;
	Layer layerManager;
	Device DeviceRef;

	VkExtent2D ScreenSize;

	VkInstance Instance;

	VkSurfaceKHR Surface;

	VkSwapchainKHR SwapChain;
	std::vector<Image> SwapChainImageViews;
	std::vector<Image> ImageResolve;

	Shader VertexShader;
	Shader FragmentShader;

	VkRenderPass RenderPass;

	std::vector<VkFramebuffer> FrameBuffers;

	VkCommandPool CommandPool;
	std::vector<VkCommandBuffer> CommandBuffers;

	std::vector<VkSemaphore> ImageAvailableSemaphore;
	std::vector<VkSemaphore> RenderFinishedSemaphore;
	std::vector<VkFence> InFlightFences;

	size_t currentFrame = 0;
	Image DepthImage;

	Object _Apple;

	Mesh _AppleMesh;
	Mesh _SponzaMesh;
	Mesh _Cube;
	Texture _AppleTexture;
	Texture _AppleSpecular;
	Texture _AppleNormal;
	CubeTexture cbt;

	Object _Skybox;

	std::shared_ptr<Material> _BasicMaterial;
	std::shared_ptr<Cubemap> _SkyboxMaterial;

	std::vector<Mesh> _SceneMeshes;
	std::vector<Object> _SceneObjects;
	std::vector<Texture> _SceneTextures;

	std::shared_ptr<Scene> _Scene;

	std::vector<VkDescriptorSet> _FrameDescriptorSets;
};