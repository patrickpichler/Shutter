#include "Texture.h"
#include "Renderer/Helpers.h"
#include <algorithm>

void Texture::Init(const Device &device, const std::string &filename, bool generateMips)
{
	_Filename = filename;

	int width;
	int height;
	int channels;

	stbi_uc* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	_Dimensions = VkExtent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

	_Buffer = Buffer(device, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, width * height * 4);
	_Buffer.Copy(device, image, width * height * 4);

	stbi_image_free(image);

	_Image.Init(
		device,
		_Dimensions,
		1,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		generateMips
	);

	CreateSampler(device);
}

void Texture::Clean(const Device &device)
{
	_Buffer.Clean(device);
	vkDestroySampler(device.GetLogicalDevice(), _Sampler, nullptr);
	_Sampler = VK_NULL_HANDLE;
	_Image.Clean(device);
}

void Texture::TransferBufferToImage(const Device &device, const VkCommandPool &cmdPool)
{

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0,0,0 };
	region.imageExtent = _Dimensions;

	_Image.TransitionLayout(device, cmdPool, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

	VkCommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(device, cmdPool);
	vkCmdCopyBufferToImage(cmdBuffer, _Buffer.GetBuffer(), _Image.GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	EndSingleUseCommandBuffer(cmdBuffer, device, cmdPool);

	if (_Image.MipLevels > 1) {
		_Image.GenerateMipmaps(device, cmdPool);
	}
	else
	{
		_Image.TransitionLayout(device, cmdPool, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}
}

void Texture::CreateSampler(const Device &device)
{
	VkSamplerCreateInfo samplerInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = _Image.MipLevels;


	vkCreateSampler(device.GetLogicalDevice(), &samplerInfo, nullptr, &_Sampler);
}