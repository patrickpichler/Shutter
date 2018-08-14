#pragma once
#include <string>
#include <array>
#include "Texture.h"

#include "Renderer/Image.h"
#include "Renderer/Buffer.h"
#include "Renderer/DeviceHandler.h"
#include "stb_image.h"

class CubeTexture: public Texture {
public:
	CubeTexture() {}
	explicit CubeTexture(Device *device) : Texture(device) {};

	void Load(const std::array<std::string, 6> &filename, bool generateMips = false);

	void TransferBufferToImage(const vk::CommandPool &cmdPool) override;

private:
	std::array<std::string, 6> _Filenames;
	std::array<Buffer, 6> _Buffers;
};
