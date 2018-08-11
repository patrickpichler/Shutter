#pragma once

#include "Buffer.h"
#include "Helpers.h"

Buffer::Buffer(
	const Device &device,
	const VkBufferUsageFlags usage,
	const size_t size,
	const VkSharingMode sharingMode
) {
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = sharingMode;

	vk_expect_success(vkCreateBuffer(device.GetLogicalDevice(), &bufferInfo, nullptr, &_Buffer), "Failed to create buffer.");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device.GetLogicalDevice(), _Buffer, &memRequirements);
	
	VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(device, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vk_expect_success(vkAllocateMemory(device.GetLogicalDevice(), &allocInfo, nullptr, &_Memory), "Failed to allocate buffer memory.");

	vk_expect_success(vkBindBufferMemory(device.GetLogicalDevice(), _Buffer, _Memory, 0), "Failed to bind the buffer and the memory.");
}

void Buffer::Copy(const Device &device, void * data, const size_t size)
{
	void *deviceData;
	vk_expect_success(vkMapMemory(device.GetLogicalDevice(), _Memory, 0, size, 0, &deviceData), "Failed to moun tthe memory for the buffer.");
	std::memcpy(deviceData, data, size);

	// Tell the driver we updated some resources
	VkMappedMemoryRange memoryRanges[] = {
		{
			VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
			nullptr,
			_Memory,
			0,
			VK_WHOLE_SIZE		/// TODO: Inspect impact of having VK_WHOLE_SIZE here
		}
	};
	
	vk_expect_success(vkFlushMappedMemoryRanges(device.GetLogicalDevice(), 1, memoryRanges), "Unable to flush mapped memory.");

	vkUnmapMemory(device.GetLogicalDevice(), _Memory);
}

void Buffer::Clean(const Device & device)
{
	if (_Memory != VK_NULL_HANDLE) {
		vkFreeMemory(device.GetLogicalDevice(), _Memory, nullptr);
		_Memory = VK_NULL_HANDLE;
	}

	if (_Buffer != VK_NULL_HANDLE) {
		vkDestroyBuffer(device.GetLogicalDevice(), _Buffer, nullptr);
		_Buffer = VK_NULL_HANDLE;
	}
}
