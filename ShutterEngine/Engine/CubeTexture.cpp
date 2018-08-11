#include "CubeTexture.h"
#include "Renderer/Helpers.h"
#include <algorithm>

void CubeTexture::Init(const Device &device, const std::array<std::string, 6> &filename, bool generateMips)
{
	_Filenames = filename;

	int width;
	int height;
	int channels;

	for (uint8_t i = 0; i < 6; ++i) {
		stbi_uc* image = stbi_load(_Filenames.at(i).c_str(), &width, &height, &channels, STBI_rgb_alpha);

		_Dimensions = VkExtent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

		_Buffers.at(i) = Buffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, width * height * 4);
		_Buffers.at(i).Copy(device, image, width * height * 4);

		stbi_image_free(image);
	}

	_Image.Init(
		device,
		_Dimensions,
		6,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		generateMips
	);

	CreateSampler(device);
}

void CubeTexture::Clean(const Device &device)
{
	//_Buffer.Clean(device);
	vkDestroySampler(device.GetLogicalDevice(), _Sampler, nullptr);
	_Sampler = VK_NULL_HANDLE;
	_Image.Clean(device);
}

void CubeTexture::TransferBufferToImage(const Device &device, const VkCommandPool &cmdPool)
{
	for (uint8_t i = 0; i < 6; ++i) {
		VkBufferImageCopy region = {};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = i;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0,0,0 };
		region.imageExtent = _Dimensions;

		_Image.TransitionLayout(device, cmdPool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

		VkCommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(device, cmdPool);
		vkCmdCopyBufferToImage(cmdBuffer, _Buffers.at(i).GetBuffer(), _Image.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
		EndSingleUseCommandBuffer(cmdBuffer, device, cmdPool);

		_Image.TransitionLayout(device, cmdPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}
