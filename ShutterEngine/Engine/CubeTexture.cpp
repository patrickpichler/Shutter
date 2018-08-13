#include "CubeTexture.h"
#include "Renderer/Helpers.h"
#include <algorithm>

void CubeTexture::Load(const std::array<std::string, 6> &filename, bool generateMips)
{
	_Filenames = filename;

	int width;
	int height;
	int channels;

	for (uint8_t i = 0; i < 6; ++i) {
		stbi_uc* image = stbi_load(_Filenames.at(i).c_str(), &width, &height, &channels, STBI_rgb_alpha);

		_Dimensions = vk::Extent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
		_Buffers.at(i) = Buffer(_Device, vk::BufferUsageFlagBits::eTransferSrc, width * height * 4);
		_Buffers.at(i).Copy(image, width * height * 4);

		stbi_image_free(image);
	}

	_Image = Image(
		_Device,
		_Dimensions,
		6,
		vk::Format::eR8G8B8A8Unorm,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		generateMips
	);

	CreateSampler();
}

void CubeTexture::TransferBufferToImage(const vk::CommandPool &cmdPool)
{
	for (uint8_t i = 0; i < 6; ++i) {
		std::array<vk::BufferImageCopy, 1> regions = {};
		regions[0].bufferOffset = 0;
		regions[0].bufferRowLength = 0;
		regions[0].bufferImageHeight = 0;
		regions[0].imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
		regions[0].imageSubresource.mipLevel = 0;
		regions[0].imageSubresource.baseArrayLayer = i;
		regions[0].imageSubresource.layerCount = 1;
		regions[0].imageOffset = { 0,0,0 };
		regions[0].imageExtent = _Dimensions;

		_Image.TransitionLayout(cmdPool, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

		vk::CommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(*_Device, cmdPool);

		cmdBuffer.copyBufferToImage(
			_Buffers.at(i).GetBuffer(),
			_Image.GetImage(),
			vk::ImageLayout::eTransferDstOptimal,
			regions
		);

		EndSingleUseCommandBuffer(cmdBuffer, *_Device, cmdPool);

		_Image.TransitionLayout(cmdPool, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
	}
}
