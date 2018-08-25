#pragma once

#define PLATFORM WIN32

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <vulkan/vulkan.hpp>
#include <Renderer/Image.h>
#include <Renderer/DeviceHandler.h>

class Surface {
public:
	Surface() {}
	Surface(Device *device, vk::Instance *instance, GLFWwindow *window);

	void Clean();

	const vk::Extent2D GetWindowDimensions() const;
	void CreateSwapChain();
	void RecreateSwapChain();

private:
	void CleanSwapChain();
	void GetSurfaceInfo();

public:
	vk::SurfaceKHR _Surface;

	vk::SwapchainKHR _Swapchain;
	std::vector<Image> _SwapchainImages;

	uint32_t _NbImages = 0;
	vk::SurfaceFormatKHR _SelectedSurfaceFormat;

private:
	Device *_Device;
	GLFWwindow *_Window;
	vk::Instance *_Instance;

	vk::SurfaceCapabilitiesKHR _SurfaceCapabilities;
	std::vector<vk::SurfaceFormatKHR> _SurfaceFormats;
};