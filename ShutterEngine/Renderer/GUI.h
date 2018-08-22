#include "imgui.h"
#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_vulkan.h"
#include <vulkan/vulkan.hpp>
#include "DeviceHandler.h"
#include "Engine/Scene.h"


class GUI {
public:
	void Init(
		Device *device,
		GLFWwindow *window,
		const vk::SurfaceKHR &surface,
		const vk::Extent2D &screenSize,
		const vk::Instance & instance,
		vk::SwapchainKHR &swapchain,
		const vk::CommandPool &cmdPool
	);

	void Render(const size_t frameId, const std::vector<vk::Framebuffer> &fb, std::vector<vk::Fence> &fence, std::vector<vk::Fence> &frameFence, std::vector<vk::Semaphore> &semaphore, Scene *scene);
	vk::RenderPass _RenderPass;
private:
	void CreateRenderPass();
	void CreateDescriptorPool();
	void CreateCommandBuffers(const vk::CommandPool &_CommandPool);

	Device * _Device;
	std::vector<vk::CommandBuffer> _CommandBuffers;

	vk::DescriptorPool _DescriptorPool;

	int selectIndex = 0;

};