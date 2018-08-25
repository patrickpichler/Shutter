#pragma once
#include <vector>
#include <memory>
#define NOMINMAX
#include "Shader.h"
#include "Extensions.h"
#include "Layers.h"
#include "DeviceHandler.h"
#include "Engine/Mesh.h"

#include "Engine/Object.h"

#include "Material.h"

#define VULKAN_VERSION VK_API_VERSION_1_0

#define GLM_FORCE_RADIANS
#define GLM_DEPTH_ZERO_TO_ONE
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
#include "GUI/GUI.h"
#include <chrono>
#include "Surface.h"

class Renderer {
public:
	Renderer(){
	}

	void Init(GLFWwindow* window, const uint16_t width, const uint16_t height, Scene *scene);
	void Draw();
	void Clean();

	void WaitIdle();

	void ReloadShaders();
	void Resize();

private:
	void CreateInstance();
	void CreateDevice();
	void CreateRenderPass();
	void CreateShadowRenderPass();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateDepth();
	void CreateOffscreen();
	void CreateCommandBuffers();
	void BuildShadowCommandBuffers();
	void BuildCommandBuffers();
	void CreateSemaphores();
private:
	// Screen/window related
	GLFWwindow *_Window;
	vk::Extent2D _ScreenSize;
	Surface _Surface;

	// Rendering related
	Extension _ExtensionManager;
	Layer _LayerManager;

	Device _Device;
	vk::Instance _Instance;

	std::vector<Image> _ImageColor;
	Image _DepthImage;
	Image _ShadowImage;
	Texture _ShadowTexture;
	bool _UpdateShadow = true;

	std::vector<vk::Framebuffer> _ShadowFramebuffer;
	std::vector<vk::Framebuffer> _Framebuffers;
	std::vector<vk::Framebuffer> _FramebuffersPresent;

	vk::RenderPass _ShadowRenderPass;
	vk::RenderPass _RenderPass;

	vk::CommandPool _CommandPool;
	std::vector<vk::CommandBuffer> _CommandBuffers;
	std::vector<vk::CommandBuffer> _ShadowCommandBuffers;

	size_t _CurrentFrame = 0;

	// Sync related
	std::vector<vk::Semaphore> _ImageAvailableSemaphore;
	std::vector<vk::Semaphore> _RenderFinishedSemaphore;
	std::vector<vk::Fence> _InFlightFences;
	std::vector<vk::Fence> _OffscreenFences;
	std::vector<vk::Fence> _ShadowFences;

	// Scene/Objects related
	Scene *_Scene;

	GUI _GUI;

	long long _FrameDuration;
	std::chrono::steady_clock::time_point start;
};