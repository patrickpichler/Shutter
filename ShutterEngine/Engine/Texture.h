#pragma once
#include <string>

#include "Renderer/Image.h"
#include "Renderer/Buffer.h"
#include "Renderer/DeviceHandler.h";
#include "stb_image.h"

class Texture {
public:
	void Init(const Device &device, const std::string &filename, bool generateMips = false);
	void Clean(const Device &device);

	void TransferBufferToImage(const Device &device, const VkCommandPool &cmdPool);

	const VkSampler &GetSampler() const {
		return _Sampler;
	}
	const Image &GetImage() const {
		return _Image;
	}
	const std::string &GetFilename() {
		return _Filename;
	}

protected:
	void CreateSampler(const Device &device);

protected:

	Image _Image;

	VkSampler _Sampler;

	VkExtent3D _Dimensions;

private:
	std::string _Filename;
	Buffer _Buffer;
};
