#pragma once
#include <vector>
#include <set>
#include <map>
#include <optional>

#include <vulkan/vulkan.hpp>

typedef std::optional<std::reference_wrapper<vk::SurfaceKHR>> optional_surface;

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
	vk::Queue VulkanQueue;
};

class Device {
public:
	Device() {}
	explicit Device(const vk::PhysicalDevice &physicalDevice);

	// Return a device with the first suitable Vulkan physical device
	static Device GetDevice(const vk::Instance &instance, const DeviceRequestInfo& info, optional_surface surface);

	void Init(DeviceRequestInfo& info, optional_surface surface);
	void Clean();


	std::set<uint32_t> GetQueueIndexSet();

	const bool IsSuitable(const DeviceRequestInfo& info, optional_surface surface) const;

	void StartMarker(const vk::CommandBuffer &cmdBuffer, const std::string &name);
	void EndMarker(const vk::CommandBuffer &cmdBuffer);


	const vk::Device& GetDevice() const {
		return _Device;
	}

	const vk::Device& operator()() const {
		return _Device;
	}

	const vk::PhysicalDevice& GetPhysicalDevice() const {
		return _PhysicalDevice;
	}

	const Queue& GetQueue(const E_QUEUE_TYPE queueType) const {
		return _Queues.at(queueType);
	}

	const vk::PhysicalDeviceProperties &GetProperties() const {
		return _PhysicalDeviceProperties;
	}

private:
	void PickQueueFamilyIndex(const DeviceRequestInfo& info, optional_surface surface);

private:
	// Physical device (graphics card)
	vk::PhysicalDevice _PhysicalDevice;
	vk::PhysicalDeviceProperties _PhysicalDeviceProperties;
	vk::PhysicalDeviceFeatures _PhysicalDeviceFeatures;

	// Logical device (vulkan handle)
	vk::Device _Device;
	std::vector<vk::ExtensionProperties> _DeviceExtensions;

	// Queues currently on the device
	std::vector<vk::QueueFamilyProperties> _QueueFamilyProperties;
	std::map<E_QUEUE_TYPE, Queue> _Queues;

	bool _SupportDebugMarkers = false;
	PFN_vkCmdDebugMarkerBeginEXT pfnCmdDebugMarkerBegin;
	PFN_vkCmdDebugMarkerEndEXT  pfnCmdDebugMarkerEnd;
};