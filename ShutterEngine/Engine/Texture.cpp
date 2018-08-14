#include "Texture.h"
#include "Renderer/Helpers.h"
#include <algorithm>

void Texture::Load(const std::string &filename, bool generateMips)
{
	_Filename = filename;

	int width;
	int height;
	int channels;

	// Load the image from disk
	stbi_uc* image = stbi_load(filename.c_str(), &width, &height, &channels, STBI_rgb_alpha);

	_Dimensions = vk::Extent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

	// Copy the data to the buffer
	_Buffer = Buffer(_Device, vk::BufferUsageFlagBits::eTransferSrc, width * height * 4);
	_Buffer.Copy(image, width * height * 4);

	stbi_image_free(image);

	_Image = Image(
		_Device,
		_Dimensions,
		1,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		generateMips
	);

	CreateSampler();
}

void Texture::Clean()
{
	//_Buffer.Clean();
	//_Device->GetDevice().destroySampler(_Sampler);
	//_Image.Clean();
}

void Texture::TransferBufferToImage(const vk::CommandPool &cmdPool)
{
	std::array<vk::BufferImageCopy, 1> regions = {};
	regions[0].bufferOffset = 0;
	regions[0].bufferRowLength = 0;
	regions[0].bufferImageHeight = 0;
	regions[0].imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
	regions[0].imageSubresource.mipLevel = 0;
	regions[0].imageSubresource.baseArrayLayer = 0;
	regions[0].imageSubresource.layerCount = 1;
	regions[0].imageOffset = { 0,0,0 };
	regions[0].imageExtent = _Dimensions;

	_Image.TransitionLayout(cmdPool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	vk::CommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(*_Device, cmdPool);

	cmdBuffer.copyBufferToImage(
		_Buffer.GetBuffer(),
		_Image.GetImage(),
		vk::ImageLayout::eTransferDstOptimal,
		regions
	);

	EndSingleUseCommandBuffer(cmdBuffer, *_Device, cmdPool);

	if (_Image.GetMipLevel() > 1u) {
		_Image.GenerateMipmaps(cmdPool);
	}
	else
	{
		_Image.TransitionLayout(cmdPool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	_Buffer.Clean();
}

void Texture::CreateSampler()
{
	vk::SamplerCreateInfo samplerInfo = {};
	samplerInfo.magFilter = vk::Filter::eLinear;
	samplerInfo.minFilter = vk::Filter::eLinear;
	samplerInfo.addressModeU = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeV = vk::SamplerAddressMode::eRepeat;
	samplerInfo.addressModeW = vk::SamplerAddressMode::eRepeat;
	samplerInfo.anisotropyEnable = false;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
	samplerInfo.unnormalizedCoordinates = false;
	samplerInfo.compareEnable = false;
	samplerInfo.compareOp = vk::CompareOp::eAlways;
	samplerInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = _Image.GetMipLevel();

	_Sampler = _Device->GetDevice().createSampler(samplerInfo);
}