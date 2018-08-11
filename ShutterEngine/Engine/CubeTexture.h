#pragma once
#include <string>
#include <array>
#include "Texture.h"

#include "Renderer/Image.h"
#include "Renderer/Buffer.h"
#include "Renderer/DeviceHandler.h";
#include "stb_image.h"

class CubeTexture: public Texture {
public:
	void Init(const Device &device, const std::array<std::string, 6> &filename, bool generateMips = false);
	void Clean(const Device &device);

	void TransferBufferToImage(const Device &device, const VkCommandPool &cmdPool);

	const VkSampler &GetSampler() const {
		return _Sampler;
	}
	const Image &GetImage() const {
		return _Image;
	}
	//const std::string &GetFilename() {
	//	return _Filename;
	//}

private:
	std::array<std::string, 6> _Filenames;
	std::array<Buffer, 6> _Buffers;
};
