#pragma once

#include <vulkan/vulkan.h>
#include "DeviceHandler.h"

class Image {
public:
	// Create an image and its image view
	void Init(
		const Device &device,
		const VkExtent3D &dimensions,
		const uint8_t layers,
		const VkFormat &format,
		const VkImageUsageFlags &usage,
		const bool generateMips = false,
		const VkSampleCountFlagBits numSamples = VK_SAMPLE_COUNT_1_BIT
	);

	// Only create the image view, based on the provided image
	void Init(
		const Device &device,
		const VkImage &image,
		const VkFormat &format
	);
	void Clean(const Device &device);

	void Copy(const VkBuffer &buffer, const uint32_t width, const uint32_t height);
	void GenerateMipmaps(const Device &device, const VkCommandPool &cmdPool);
	void TransitionLayout(const Device &device, const VkCommandPool &cmdPool, const VkImageLayout oldLayout, const VkImageLayout newLayout);


	const VkImage &GetImage() const {
		return ImageHolder;
	}

	const VkImageView &GetImageView() const {
		return ImageView;
	}

	const VkFormat &GetFormat() const {
		return Format;
	}

	uint8_t MipLevels;

private:
	void CreateImage(const Device &device);
	void AllocateMemory(const Device &device);
	void CreateImageView(const Device &device);

private:
	VkFormat Format;
	VkImageUsageFlags Usage;
	VkExtent3D Dimensions;
	uint8_t Layers;

	VkSampleCountFlagBits NumSamples;

	VkImage ImageHolder;
	VkDeviceMemory DeviceMemory;

	VkImageView ImageView;
};