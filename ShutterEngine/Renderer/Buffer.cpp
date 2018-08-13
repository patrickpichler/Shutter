#pragma once

#include "Buffer.h"
#include "Helpers.h"
#include <iostream>


Buffer::Buffer(
	Device *device,
	const vk::BufferUsageFlagBits usage,
	const size_t size,
	const vk::SharingMode sharingMode
):
	_Device(device)
{
	std::cerr << "Allocating: " << size << std::endl;

	_Buffer = _Device->GetDevice().createBuffer(vk::BufferCreateInfo( {}, size, usage, sharingMode ));

	vk::MemoryRequirements memoryRequirements = _Device->GetDevice().getBufferMemoryRequirements(_Buffer);

	vk::MemoryAllocateInfo memoryInfo;
	memoryInfo.allocationSize = memoryRequirements.size;
	memoryInfo.memoryTypeIndex = findMemoryType(*_Device, memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	_Memory = _Device->GetDevice().allocateMemory(memoryInfo);
	_Device->GetDevice().bindBufferMemory(_Buffer, _Memory, 0);
}

void Buffer::Copy(void * data, const size_t size)
{
	void *deviceData;
	deviceData = _Device->GetDevice().mapMemory(_Memory, 0, size, {});
	std::memcpy(deviceData, data, size);

	std::array<vk::MappedMemoryRange, 1> memoryRanges;
	memoryRanges[0] = vk::MappedMemoryRange(_Memory, 0, VK_WHOLE_SIZE);
	_Device->GetDevice().flushMappedMemoryRanges(memoryRanges);

	_Device->GetDevice().unmapMemory(_Memory);
}

void Buffer::Clean()
{
	//if (_Memory) {
	//	_Device->GetDevice().freeMemory(_Memory);
	//}

	//if (_Buffer) {
	//	_Device->GetDevice().destroyBuffer(_Buffer);
	//}

	//_Device = nullptr;
}
