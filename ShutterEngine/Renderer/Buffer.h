#pragma once

#include <vulkan/vulkan.hpp>
#include "DeviceHandler.h"
#include <algorithm>

class Buffer {
public:
	Buffer(){}
	explicit Buffer(
		Device *device,
		const vk::BufferUsageFlagBits usage,
		const size_t size,
		const vk::SharingMode sharingMode = vk::SharingMode::eExclusive
	);
	//Buffer(const Buffer& buffer) :
	//	_Device(buffer._Device),
	//	_Buffer(buffer._Buffer),
	//	_Memory(buffer._Memory)
	//{

	//}

	//Buffer& operator=(Buffer buffer)
	//{
	//	_Device = buffer._Device;
	//	std::swap(_Buffer, buffer._Buffer);
	//	std::swap(_Memory, buffer._Memory);

	//	return *this;
	//}

	~Buffer() {
		Clean();
	}

	void Copy(void *data, const size_t size);

	void Clean();


	const vk::Buffer &GetBuffer() const {
		return _Buffer;
	}

	const vk::DeviceMemory &GetMemory() const {
		return _Memory;
	}

protected:
	Device *_Device;

	vk::Buffer _Buffer;
	vk::DeviceMemory _Memory;
};