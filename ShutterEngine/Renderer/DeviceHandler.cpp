#include "DeviceHandler.h"
#include <algorithm>

#include "Helpers.h"

Device::Device(const vk::PhysicalDevice & physicalDevice):
	_PhysicalDevice(physicalDevice)
{
	_PhysicalDeviceProperties = _PhysicalDevice.getProperties();
	_PhysicalDeviceFeatures = _PhysicalDevice.getFeatures();

	_DeviceExtensions = _PhysicalDevice.enumerateDeviceExtensionProperties();

	_QueueFamilyProperties = _PhysicalDevice.getQueueFamilyProperties();
}

void Device::Init(const DeviceRequestInfo& info, optional_surface surface)
{
	PickQueueFamilyIndex(info, surface);

	std::vector<vk::DeviceQueueCreateInfo> deviceQueuesInfo;
	for (const auto& id : GetQueueIndexSet()) {
		float queuePrio = 1.0f;
		deviceQueuesInfo.push_back(vk::DeviceQueueCreateInfo{ {}, id, 1, &queuePrio });
	}

	vk::PhysicalDeviceFeatures deviceFeatures = {};

	vk::DeviceCreateInfo deviceInfo = {};
	deviceInfo.pQueueCreateInfos = deviceQueuesInfo.data();
	deviceInfo.queueCreateInfoCount = deviceQueuesInfo.size();
	deviceInfo.pEnabledFeatures = &deviceFeatures;
	deviceInfo.enabledExtensionCount = info.RequiredExtensions.size();
	deviceInfo.ppEnabledExtensionNames = info.RequiredExtensions.data();

	_Device = _PhysicalDevice.createDevice(deviceInfo);

	for (auto &queue : _Queues)
	{
		queue.second.VulkanQueue = _Device.getQueue(queue.second.Index, 0);
	}
}

void Device::Clean()
{
	//_Device.destroy();
}

std::set<uint32_t> Device::GetQueueIndexSet()
{
	std::set<uint32_t> uniqueQueueId;
	for (const auto &queue : _Queues)
	{
		uniqueQueueId.insert(queue.second.Index);
	}

	return uniqueQueueId;
}

const bool Device::IsSuitable(const DeviceRequestInfo& info, optional_surface surface) const
{
	// Check that we got all the required extensions
	for (const auto& extensionName : info.RequiredExtensions) {
		auto it = std::find_if(
			_DeviceExtensions.begin(),
			_DeviceExtensions.end(),
			[&extensionName](const vk::ExtensionProperties&  availableExtenion) {
				return std::strcmp(availableExtenion.extensionName, extensionName) == 0;
			}
		);

		if (it == _DeviceExtensions.end()) {
			return false;
		}
	}

	// Check that we have at least a queue
	if (_QueueFamilyProperties.size() == 0) {
		return false;
	}

	// Check the queues for the required setup
	bool havePresentation = !info.SupportPresentation;
	bool haveGraphics = !info.SupportGraphics;
	uint32_t i = 0;
	for (const auto &queueFamily : _QueueFamilyProperties) {
		if (info.SupportPresentation && surface.has_value()) {
			vk::Bool32 presentSupport = _PhysicalDevice.getSurfaceSupportKHR(i, surface.value());

			if (presentSupport) {
				havePresentation = true;
			}
		}
		if (info.SupportGraphics) {
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				haveGraphics = true;
			}
		}
		++i;
	}

	return havePresentation && haveGraphics;
}

void Device::PickQueueFamilyIndex(const DeviceRequestInfo& info, optional_surface surface)
{
	// Look for a queue with present if we need one
	uint32_t i = 0;
	if (info.SupportPresentation && surface.has_value()) {
		for (const auto &queueFamily : _QueueFamilyProperties) {

			vk::Bool32 presentSupport = _PhysicalDevice.getSurfaceSupportKHR(i, surface.value());

			if (presentSupport) {
				Queue present = { i };
				_Queues.emplace(PRESENT, present);
				break;
			}
		}
	}

	// Look for a queue sith graphics if we need one
	i = 0;
	if (info.SupportGraphics) {
		for (const auto &queueFamily : _QueueFamilyProperties) {
			if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
				Queue present = { i };
				_Queues.emplace(GRAPHICS, present);
				break;
			}
		}
		++i;
	}
}

Device Device::GetDevice(const vk::Instance &instance, const DeviceRequestInfo& info, optional_surface surface)
{
	uint32_t deviceCount = 0;
	std::vector<vk::PhysicalDevice> physicalDeviceList;
	physicalDeviceList = instance.enumeratePhysicalDevices();

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
