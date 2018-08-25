#pragma once

#include <vulkan/vulkan.h>
#include "DeviceHandler.h"

class Image {
public:
	Image(){}
	explicit Image(
		Device *device,
		const VkExtent3D &dimensions,
		const uint8_t layers,
		const vk::Format &format,
		const vk::ImageUsageFlags usage,
		const bool generateMips = false,
		const vk::SampleCountFlagBits numSamples = vk::SampleCountFlagBits::e1
	);

	// Only create the image view, based on the provided image
	void FromVkImage(
		Device *device,
		const vk::Image &image,
		const vk::Format &format
	);
	void Clean();

	void GenerateMipmaps(const vk::CommandPool &cmdPool);
	void TransitionLayout(const vk::CommandPool &cmdPool, const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout);


	const vk::Image &GetImage() const {
		return _Image;
	}

	const vk::ImageView &GetImageView() const {
		return _View;
	}

	const vk::Format &GetFormat() const {
		return _Format;
	}

	const uint32_t GetMipLevel() const {
		return _MipLevels;
	}

private:
	void CreateImage();
	void AllocateMemory();
	void CreateImageView();

private:
	Device *_Device;

	vk::Format _Format;
	vk::ImageUsageFlags _Usage;
	vk::SampleCountFlagBits _NumSamples;

	vk::Extent3D _Dimensions;
	uint32_t _MipLevels;
	uint32_t _NbLayers;

	vk::Image _Image;
	vk::ImageView _View;

	vk::DeviceMemory _Memory;

	bool _FromSwapchain = false;
};