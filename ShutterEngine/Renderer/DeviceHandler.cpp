#include "DeviceHandler.h"
#include <algorithm>

#include "Helpers.h"

Device::Device(const VkPhysicalDevice & physicalDevice):
	VulkanDevice(physicalDevice)
{
	vkGetPhysicalDeviceProperties(VulkanDevice, &DeviceProperties);
	vkGetPhysicalDeviceFeatures(VulkanDevice, &DeviceFeatures);

	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
	DeviceExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, DeviceExtensions.data());

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(VulkanDevice, &queueFamilyCount, nullptr);
	QueueFamilies.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(VulkanDevice, &queueFamilyCount, QueueFamilies.data());
}

void Device::Init(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface)
{
	PickQueueFamilyIndex(info, surface);

	std::vector<VkDeviceQueueCreateInfo> deviceQueuesInfo;
	for (const auto& id : GetQueueIndexSet()) {
		float queuePrio = 1.0f;
		VkDeviceQueueCreateInfo queueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueFamilyIndex = id;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePrio;
		deviceQueuesInfo.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = {};

	VkDeviceCreateInfo deviceInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	deviceInfo.pQueueCreateInfos = deviceQueuesInfo.data();
	deviceInfo.queueCreateInfoCount = deviceQueuesInfo.size();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = info.RequiredExtensions.size();
	deviceInfo.ppEnabledExtensionNames = info.RequiredExtensions.data();

	vk_expect_success(vkCreateDevice(VulkanDevice, &deviceInfo, nullptr, &LogicalDevice), "Fail to create logical device.");

	for (auto &queue : QueueList)
	{
		vkGetDeviceQueue(LogicalDevice, queue.second.Index, 0, &queue.second.VulkanQueue);
	}
}

void Device::Clean()
{
	vkDestroyDevice(LogicalDevice, nullptr);
}

/// DEPRECATED
const VkDevice & Device::GetLogicalDevice() const
{
	return LogicalDevice;
}

vk::Device & Device::GetLogicalDeviceNew() const
{
	return vk::Device(LogicalDevice);
}

const VkPhysicalDevice& Device::GetPhysicalDevice() const
{
	return VulkanDevice;
}

const Queue& Device::GetQueue(const E_QUEUE_TYPE queueType) const
{
	return QueueList.at(queueType);
}

std::set<uint32_t> Device::GetQueueIndexSet()
{
	std::set<uint32_t> uniqueQueueId;
	for (const auto &queue : QueueList)
	{
		uniqueQueueId.insert(queue.second.Index);
	}

	return uniqueQueueId;
}

const bool Device::IsSuitable(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface) const
{
	// Check that we got all the required extensions
	for (const auto& extensionName : info.RequiredExtensions) {
		auto it = std::find_if(
			DeviceExtensions.begin(),
			DeviceExtensions.end(),
			[&extensionName](const VkExtensionProperties&  availableExtenion) {
				return std::strcmp(availableExtenion.extensionName, extensionName) == 0;
			}
		);

		if (it == DeviceExtensions.end()) {
			return false;
		}
	}

	// Check that we at least have a queue
	if (QueueFamilies.size() == 0) {
		return false;
	}

	// Check the queues for the required setup
	bool havePresentation = !info.SupportPresentation;
	bool haveGraphics = !info.SupportGraphics;
	uint32_t i = 0;
	for (const auto &queueFamily : QueueFamilies) {
		if (info.SupportPresentation && surface.has_value()) {
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(VulkanDevice, i, surface.value(), &presentSupport);

			if (presentSupport) {
				havePresentation = true;
			}
		}
		if (info.SupportGraphics) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				haveGraphics = true;
			}
		}
		++i;
	}

	return havePresentation && haveGraphics;
}

void Device::PickQueueFamilyIndex(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface)
{
	uint32_t i = 0;
	if (info.SupportPresentation && surface.has_value()) {
		for (const auto &queueFamily : QueueFamilies) {

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(VulkanDevice, i, surface.value(), &presentSupport);

			if (presentSupport) {
				Queue present = { i };
				QueueList.emplace(PRESENT, present);
				break;
			}
		}
	}

	i = 0;
	if (info.SupportGraphics) {
		for (const auto &queueFamily : QueueFamilies) {
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				Queue present = { i };
				QueueList.emplace(GRAPHICS, present);
				break;
			}
		}
		++i;
	}
}

Device DeviceHandler::GetDevice(const VkInstance& instance, const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface)
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("No Vulkan capable GPU detected");
	}

	std::vector<VkPhysicalDevice> physicalDeviceList(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDeviceList.data());

	std::vector<Device> deviceList;

	for (const auto& physicalDevice : physicalDeviceList) {
		Device device(physicalDevice);
		if (device.IsSuitable(info, surface)) {
			deviceList.push_back(device);
		}
	}

	if (deviceList.size() == 0) {
		throw std::runtime_error("No suitable GPU detected");
	}

	return deviceList.front();
}
