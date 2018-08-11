#pragma once
#include <vector>
#include <map>
#include <optional>
#include <vulkan/vulkan.hpp>
#include <set>

struct DeviceRequestInfo {
	std::vector<const char*> RequiredExtensions;
	bool SupportPresentation = false;
	bool SupportGraphics = false;
};

enum E_QUEUE_TYPE
{
	GRAPHICS,
	PRESENT
};

struct Queue {
	uint32_t Index;
	VkQueue VulkanQueue;
};

class Device {
public:
	Device() : VulkanDevice(VK_NULL_HANDLE){}
	Device(const VkPhysicalDevice &physicalDevice);

	void Init(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface);
	void Clean();

	const VkDevice& GetLogicalDevice() const;
	vk::Device& GetLogicalDeviceNew() const;
	const VkPhysicalDevice& GetPhysicalDevice() const;
	const Queue& GetQueue(const E_QUEUE_TYPE queueType) const;
	std::set<uint32_t> GetQueueIndexSet();

	const bool IsSuitable(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface) const;

	const VkPhysicalDeviceProperties &GetProperties() const {
		return DeviceProperties;
	}

private:
	void PickQueueFamilyIndex(const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface);

private:
	VkPhysicalDevice VulkanDevice;
	VkDevice LogicalDevice;

	VkPhysicalDeviceProperties DeviceProperties;
	VkPhysicalDeviceFeatures DeviceFeatures;
	std::vector<VkExtensionProperties> DeviceExtensions;
	std::vector<VkQueueFamilyProperties> QueueFamilies;

	std::map<E_QUEUE_TYPE, Queue> QueueList;
};

namespace DeviceHandler {
	Device GetDevice(const VkInstance &instance, const DeviceRequestInfo& info, std::optional<std::reference_wrapper<VkSurfaceKHR>> surface);
}