#pragma once
#include <array>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "Buffer.h"

#define vk_expect_success(func, message) { \
	if (func != VK_SUCCESS) { \
		throw std::runtime_error(message); \
	} \
}

static uint32_t findMemoryType(const Device& device, uint32_t typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memoryProperties = device.GetPhysicalDevice().getMemoryProperties();

	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

// Create a one shot command buffer
static vk::CommandBuffer BeginSingleUseCommandBuffer(const Device &device, const vk::CommandPool &cmdPool)
{
	std::vector<vk::CommandBuffer> cmdBuffer = device().allocateCommandBuffers({ cmdPool, vk::CommandBufferLevel::ePrimary, 1 });
	cmdBuffer.front().begin({ vk::CommandBufferUsageFlagBits::eOneTimeSubmit });
	return cmdBuffer.front();
}

static void EndSingleUseCommandBuffer(const vk::CommandBuffer &cmdBuffer, const Device &device, const vk::CommandPool &cmdPool)
{
	cmdBuffer.end();

	std::array<vk::SubmitInfo, 1> submitInfo;
	submitInfo[0] = {};
	submitInfo[0].commandBufferCount = 1;
	submitInfo[0].pCommandBuffers = &cmdBuffer;


	device.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue.submit(submitInfo, VK_NULL_HANDLE);
	device.GetQueue(E_QUEUE_TYPE::GRAPHICS).VulkanQueue.waitIdle();
	device().freeCommandBuffers(cmdPool, { cmdBuffer });
}

