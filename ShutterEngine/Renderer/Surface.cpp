#include "Surface.h"

Surface::Surface(Device * device, vk::Instance *instance, GLFWwindow *window) : 
	_Device(device),
	_Window(window),
	_Instance(instance)
{
	// Platform specific code for handling the surface goes here
#if PLATFORM == WIN32
	_Surface = _Instance->createWin32SurfaceKHR(vk::Win32SurfaceCreateInfoKHR(
		{},
		GetModuleHandle(nullptr),
		glfwGetWin32Window(_Window)
	));
#endif
}

void Surface::Clean()
{
	CleanSwapChain();

	_Instance->destroySurfaceKHR(_Surface);
}

const vk::Extent2D Surface::GetWindowDimensions() const
{
	int width, height;

	glfwGetWindowSize(_Window, &width, &height);

	return vk::Extent2D(width, height);
}

void Surface::CreateSwapChain()
{
	// Populate the info if they are empty
	if (_NbImages == 0) {
		GetSurfaceInfo();
	}

	// Create the swapchain
	auto& queueIndexSet = _Device->GetQueueIndexSet();
	std::vector<uint32_t> queueIndexList;
	std::copy(queueIndexSet.begin(), queueIndexSet.end(), std::back_inserter(queueIndexList));

	_Swapchain = _Device->GetDevice().createSwapchainKHR(vk::SwapchainCreateInfoKHR(
		{},
		_Surface,
		_NbImages,
		_SelectedSurfaceFormat.format,
		_SelectedSurfaceFormat.colorSpace,
		GetWindowDimensions(),
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

	// Create the swapchain images
	std::vector<vk::Image> swapchainImages(_Device->GetDevice().getSwapchainImagesKHR(_Swapchain));
	_SwapchainImages.resize(swapchainImages.size());

	size_t i = 0;
	for (auto &image : _SwapchainImages) {
		image.FromVkImage(_Device, swapchainImages[i], _SelectedSurfaceFormat.format);
		++i;
	}
}

void Surface::RecreateSwapChain()
{
	CleanSwapChain();
	_NbImages = 0;
	CreateSwapChain();
}

void Surface::CleanSwapChain()
{
	// Clean the images
	for (auto &image : _SwapchainImages) {
		image.Clean();
	}

	// Clean the swapchain
	_Device->GetDevice().destroySwapchainKHR(_Swapchain);
}

void Surface::GetSurfaceInfo()
{
	// Query the newly create surface capabilities
	_SurfaceCapabilities = _Device->GetPhysicalDevice().getSurfaceCapabilitiesKHR(_Surface);
	_SurfaceFormats = _Device->GetPhysicalDevice().getSurfaceFormatsKHR(_Surface);

	_NbImages = _SurfaceCapabilities.minImageCount;
	_SelectedSurfaceFormat = _SurfaceFormats.front();
}
