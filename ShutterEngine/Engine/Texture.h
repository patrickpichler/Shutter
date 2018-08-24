#pragma once
#include <string>

#include "Renderer/Image.h"
#include "Renderer/Buffer.h"
#include "Renderer/DeviceHandler.h"
#include "stb_image.h"

class Texture {
public:
	Texture(){}
	explicit Texture(Device *device) : _Device(device){};
	~Texture() {
		Clean();
	}

	void Load(const std::string &filename, bool generateMips = false);
	void Clean();

	virtual void TransferBufferToImage(const vk::CommandPool &cmdPool);

	const vk::Sampler &GetSampler() const {
		return _Sampler;
	}
	const Image &GetImage() const {
		return _Image;
	}
	const std::string &GetFilename() {
		return _Filename;
	}

//protected:
	Image _Image;
	void CreateSampler();

//protected:
	Device *_Device;
//public:

	vk::Sampler _Sampler;

	vk::Extent3D _Dimensions;

private:
	std::string _Filename;
	Buffer _Buffer;
};
