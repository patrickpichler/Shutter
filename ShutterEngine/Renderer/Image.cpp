#include "Image.h"
#include "Helpers.h"
#include <algorithm>

Image::Image(
	Device *device,
	const VkExtent3D &dimensions,
	const uint8_t layers,
	const vk::Format &format,
	const vk::ImageUsageFlags usage,
	const bool generateMips,
	const vk::SampleCountFlagBits numSamples
) : 
	_Device(device)
{
	_Format = format;
	_Usage = usage;
	_NbLayers = layers;
	_Dimensions = dimensions;
	_NumSamples = numSamples;

	if (generateMips) {
		_MipLevels = std::floor(std::log2(std::max(_Dimensions.width, _Dimensions.height))) + 1;
		_Usage |= vk::ImageUsageFlagBits::eTransferSrc;
	}
	else {
		_MipLevels = 1;
	}

	CreateImage();
	AllocateMemory();

	CreateImageView();
}

void Image::FromVkImage(
	Device *device,
	const vk::Image &image,
	const vk::Format &format
) {
	_Device = device;

	_Image = image;
	_Format = format;
	_Dimensions = VkExtent3D{ 0, 0, 1 };
	_NbLayers = 1;
	_MipLevels = 1;

	CreateImageView();
}

void Image::Clean()
{
	//if (_View) {
	//	_Device->GetDevice().destroyImageView(_View);
	//}

	//// In case we obtained the image through the sawpchain, do not clear it
	//if (!_Usage) {
	//	if (_Image) {
	//		_Device->GetDevice().destroyImage(_Image);
	//	}

	//	if (_Memory) {
	//		_Device->GetDevice().freeMemory(_Memory);
	//	}
	//}
}

void Image::GenerateMipmaps(const vk::CommandPool& cmdPool)
{
	vk::CommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(*_Device, cmdPool);

	std::array<vk::ImageMemoryBarrier, 1> barriers;
	barriers[0].image = _Image;
	barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barriers[0].subresourceRange.baseArrayLayer = 0;
	barriers[0].subresourceRange.layerCount = 1;
	barriers[0].subresourceRange.levelCount = 1;

	int32_t mipWidth = _Dimensions.width;
	int32_t mipHeight = _Dimensions.height;



	for (uint32_t i = 1; i < _MipLevels; ++i) {
		barriers[0].subresourceRange.baseMipLevel = i - 1;
		barriers[0].oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barriers[0].newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eTransferRead;

		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eTransfer,
			vk::DependencyFlags(),
			nullptr,
			nullptr,
			barriers[0]
		);

		vk::ImageBlit blit(
			vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor,i-1, 0, 1),
			{
				vk::Offset3D{0,0,0}, 
				vk::Offset3D{ mipWidth, mipHeight, 1}
			},
			vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, i, 0, 1),
			{
				vk::Offset3D{ 0,0,0},
				vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 }
			}
		);

		cmdBuffer.blitImage(
			_Image, vk::ImageLayout::eTransferSrcOptimal,
			_Image, vk::ImageLayout::eTransferDstOptimal,
			{ blit },
			vk::Filter::eLinear
		);

		barriers[0].oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barriers[0].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;

		cmdBuffer.pipelineBarrier(
			vk::PipelineStageFlagBits::eTransfer,
			vk::PipelineStageFlagBits::eFragmentShader,
			vk::DependencyFlags(),
			nullptr,
			nullptr,
			barriers[0]
		);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}


	barriers[0].subresourceRange.baseMipLevel = _MipLevels - 1;
	barriers[0].oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barriers[0].newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferRead;
	barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;

	cmdBuffer.pipelineBarrier(
		vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eFragmentShader,
		vk::DependencyFlags(),
		nullptr,
		nullptr,
		barriers[0]
	);

	EndSingleUseCommandBuffer(cmdBuffer, *_Device, cmdPool);
}

void Image::TransitionLayout(const vk::CommandPool &cmdPool, const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout)
{
	vk::CommandBuffer cmdBuffer = BeginSingleUseCommandBuffer(*_Device, cmdPool);

	std::array<vk::ImageMemoryBarrier, 1> barriers;
	barriers[0].image = _Image;
	barriers[0].oldLayout = oldLayout;
	barriers[0].newLayout = newLayout;
	barriers[0].srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barriers[0].subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	barriers[0].subresourceRange.baseArrayLayer = 0;
	barriers[0].subresourceRange.layerCount = _NbLayers;
	barriers[0].subresourceRange.levelCount = _MipLevels;

	vk::PipelineStageFlagBits srcStage, dstStage;

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barriers[0].srcAccessMask = {};
		barriers[0].dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if(oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barriers[0].srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barriers[0].dstAccessMask = vk::AccessFlagBits::eShaderRead;

		srcStage = vk::PipelineStageFlagBits::eTransfer;
		dstStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		barriers[0].srcAccessMask = {};
		barriers[0].dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;

		srcStage = vk::PipelineStageFlagBits::eTopOfPipe;
		dstStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
	}

	cmdBuffer.pipelineBarrier(
		srcStage,
		dstStage,
		vk::DependencyFlags(),
		nullptr,
		nullptr,
		barriers[0]
	);

	EndSingleUseCommandBuffer(cmdBuffer, *_Device, cmdPool);
}

void Image::CreateImage()
{
	vk::ImageCreateFlagBits flags = {};

	if (_NbLayers > 1) {
		flags = vk::ImageCreateFlagBits::eCubeCompatible;
	}

	_Image = _Device->GetDevice().createImage(vk::ImageCreateInfo(
		flags,
		_Dimensions.depth > 1 ? vk::ImageType::e3D : vk::ImageType::e2D,
		_Format,
		_Dimensions,
		_MipLevels,
		_NbLayers,
		_NumSamples,
		vk::ImageTiling::eOptimal,
		_Usage,
		vk::SharingMode::eExclusive
	));
}

void Image::AllocateMemory()
{
	vk::MemoryRequirements imageMemReq = _Device->GetDevice().getImageMemoryRequirements(_Image);

	_Memory = _Device->GetDevice().allocateMemory(
		vk::MemoryAllocateInfo(
			imageMemReq.size,
			findMemoryType(*_Device, imageMemReq.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal)
		)
	);

	_Device->GetDevice().bindImageMemory(_Image, _Memory, 0);
}

void Image::CreateImageView()
{
	// Get the right image type
	vk::ImageViewType imageType;
	if (_Dimensions.depth > 1) {
		imageType = vk::ImageViewType::e3D;
	}
	else if (_NbLayers > 1) {
		imageType = vk::ImageViewType::eCube;
	}
	else {
		imageType = vk::ImageViewType::e2D;
	}

	_View = _Device->GetDevice().createImageView(vk::ImageViewCreateInfo(
		vk::ImageViewCreateFlags(),
		_Image,
		imageType,
		_Format,
		vk::ComponentMapping(),
		vk::ImageSubresourceRange(
			_Usage == vk::ImageUsageFlagBits::eDepthStencilAttachment ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor,
			0,
			_MipLevels,
			0,
			_NbLayers
		)
	));
}
