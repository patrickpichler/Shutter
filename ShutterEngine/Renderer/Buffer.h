#pragma once

#include <vulkan/vulkan.hpp>
#include "DeviceHandler.h"

class Buffer {
public:
	Buffer(){}
	Buffer(
		const Device &device,
		const VkBufferUsageFlags usage,
		const size_t size,
		const VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE
	);

	void Copy(const Device &device, void *data, const size_t size);

	void Clean(const Device &device);

	/// Deprecated
	const VkBuffer &GetBuffer() const {
		return _Buffer;
	}

	const vk::Buffer &GetBufferNew() const {
		return vk::Buffer(_Buffer);
	}

	const VkDeviceMemory &GetMemory() const {
		return _Memory;
	}

protected:
	VkBuffer _Buffer;
	VkDeviceMemory _Memory;
};