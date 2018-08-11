#include "Image.h"
#include "Helpers.h"
#include <algorithm>

void Image::Init(
	const Device &device,
	const VkExtent3D &dimensions,
	const uint8_t layers,
	const VkFormat &format, 
	const VkImageUsageFlags &usage,
	const bool generateMips,
	const VkSampleCountFlagBits numSamples
)
{
	Format = format;
	Usage = usage;
	Layers = layers;
	Dimensions = dimensions;
	NumSamples = numSamples;

	if (generateMips) {
		MipLevels = std::floor(std::log2(std::max(Dimensions.width, Dimensions.height))) + 1;
		Usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}
	else {
		MipLevels = 1;
	}

	CreateImage(device);
	AllocateMemory(device);

	CreateImageView(device);
}

void Image::Init(const Device &device, const VkImage &image, const VkFormat &format)
{
	ImageHolder = image;
	Format = format;
	Dimensions = VkExtent3D{ 0, 0, 1 };
	Layers = 1;
	MipLevels = 1;

	CreateImageView(device);
}

void Image::Clean(const Device &device)
{
	if (ImageView != VK_NULL_HANDLE) {
		vkDestroyImageView(device.GetLogicalDevice(), ImageView, nullptr);
		ImageView = VK_NULL_HANDLE;
	}

	// In case we obtained the image through the sawpchain, do not clear it
	if (Usage != VK_NULL_HANDLE) {
		if (ImageHolder != VK_NULL_HANDLE) {
			vkDestroyImage(device.GetLogicalDevice(), ImageHolder, nullptr);
			ImageHolder = VK_NULL_HANDLE;
		}

		if (DeviceMemory != VK_NULL_HANDLE) {
			vkFreeMemory(device.GetLogicalDevice(), DeviceMemory, nullptr);
			DeviceMemory = VK_NULL_HANDLE;
		}
	}
}

void Image::Copy(const VkBuffer & buffer, const uint32_t width, const uint32_t height)
{
}

void Image::GenerateMipmaps(const Device &device, const VkCommandPool & cmdPool)
{
	VkCommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(device, cmdPool);

	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.image = ImageHolder;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = Dimensions.width;
	int32_t mipHeight = Dimensions.height;

	for (uint32_t i = 1; i < MipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		VkImageBlit blit = {};
		blit.srcOffsets[0] = { 0, 0, 0 };
		blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[0] = { 0, 0, 0 };
		blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(cmdBuffer,
			ImageHolder, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			ImageHolder, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(cmdBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = MipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(cmdBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier);

	EndSingleUseCommandBuffer(cmdBuffer, device, cmdPool);
}

void Image::TransitionLayout(const Device &device, const VkCommandPool &cmdPool, const VkImageLayout oldLayout, const VkImageLayout newLayout)
{
	VkCommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(device, cmdPool);

	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = ImageHolder;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = MipLevels;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = Layers;

	VkPipelineStageFlags srcStage, dstStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}

	vkCmdPipelineBarrier(cmdBuffer, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

	EndSingleUseCommandBuffer(cmdBuffer, device, cmdPool);
}

void Image::CreateImage(const Device &device)
{
	VkImageCreateInfo imageInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = Dimensions.depth > 1 ? VK_IMAGE_TYPE_3D : VK_IMAGE_TYPE_2D;
	imageInfo.extent = Dimensions;
	imageInfo.mipLevels = MipLevels;
	imageInfo.arrayLayers = Layers;
	imageInfo.format = Format;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = Usage;
	imageInfo.samples = NumSamples;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (Layers > 1) {
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	}

	vk_expect_success(vkCreateImage(device.GetLogicalDevice(), &imageInfo, nullptr, &ImageHolder), "Failed to create image.");
}

void Image::AllocateMemory(const Device &device)
{
	VkMemoryRequirements imageMemReq;
	vkGetImageMemoryRequirements(device.GetLogicalDevice(), ImageHolder, &imageMemReq);

	VkMemoryAllocateInfo memAllocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memAllocInfo.allocationSize = imageMemReq.size;
	memAllocInfo.memoryTypeIndex = findMemoryType(device, imageMemReq.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vk_expect_success(vkAllocateMemory(device.GetLogicalDevice(), &memAllocInfo, nullptr, &DeviceMemory), "Failed to allocate memory for the image.");

	vkBindImageMemory(device.GetLogicalDevice(), ImageHolder, DeviceMemory, 0);
}

void Image::CreateImageView(const Device & device)
{
	// Get the right image type
	VkImageViewType imageType;
	if (Dimensions.depth > 1) {
		imageType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else if (Layers > 1) {
		imageType = VK_IMAGE_VIEW_TYPE_CUBE;
	}
	else {
		imageType = VK_IMAGE_VIEW_TYPE_2D;
	}

	VkImageViewCreateInfo imageViewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewInfo.image = ImageHolder;
	imageViewInfo.viewType = imageType;
	imageViewInfo.format = Format;
	imageViewInfo.subresourceRange.aspectMask = Usage == VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = MipLevels;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = Layers;

	vk_expect_success(vkCreateImageView(device.GetLogicalDevice(), &imageViewInfo, nullptr, &ImageView), "Failed to create the image view.");
}
