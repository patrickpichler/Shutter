#pragma once
#include <vulkan/vulkan.h>
#include "Buffer.h"
#include <glm/glm.hpp>

#define vk_expect_success(func, message) { \
	if (func != VK_SUCCESS) { \
		throw std::runtime_error(message); \
	} \
}

static uint32_t findMemoryType(const Device& device, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(device.GetPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

// Create a one shot command buffer
static VkCommandBuffer BeginSingleUseCommandBuffer(const Device &device, const VkCommandPool &cmdPool)
{
	VkCommandBufferAllocateInfo cmdBufferAllocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufferAllocInfo.commandPool = cmdPool;
	cmdBufferAllocInfo.commandBufferCount = 1;

	VkCommandBuffer cmdBuffer;
	vkAllocateCommandBuffers(device.GetLogicalDevice(), &cmdBufferAllocInfo, &cmdBuffer);

	VkCommandBufferBeginInfo cmdBufferBegin = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	cmdBufferBegin.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(cmdBuffer, &cmdBufferBegin);
	return cmdBuffer;
}

static void EndSingleUseCommandBuffer(const VkCommandBuffer cmdBuffer, const Device &device, const VkCommandPool &cmdPool)
{
	vkEndCommandBuffer(cmdBuffer);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	vkQueueSubmit(device.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(device.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue);

	vkFreeCommandBuffers(device.GetLogicalDevice(), cmdPool, 1, &cmdBuffer);
}

