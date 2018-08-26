#pragma once

#include "Buffer.h"
#include "Helpers.h"
#include <iostream>


Buffer::Buffer(
	Device *device,
	const vk::BufferUsageFlagBits usage,
	const size_t size,
	const vk::SharingMode sharingMode,
	vk::MemoryPropertyFlags memoryFlags
):
	_Device(device)
{
	std::cerr << "Allocating: " << size << std::endl;

	_Buffer = _Device->GetDevice().createBuffer(vk::BufferCreateInfo( {}, size, usage, sharingMode ));

	vk::MemoryRequirements memoryRequirements = _Device->GetDevice().getBufferMemoryRequirements(_Buffer);

	vk::MemoryAllocateInfo memoryInfo;
	memoryInfo.allocationSize = memoryRequirements.size;
	memoryInfo.memoryTypeIndex = findMemoryType(*_Device, memoryRequirements.memoryTypeBits, memoryFlags);

	_Memory = _Device->GetDevice().allocateMemory(memoryInfo);
	_Device->GetDevice().bindBufferMemory(_Buffer, _Memory, 0);
}

void Buffer::Copy(void * data, const size_t size)
{
	_Size = size;
	void *deviceData;
	deviceData = _Device->GetDevice().mapMemory(_Memory, 0, size, {});
	std::memcpy(deviceData, data, size);

	std::array<vk::MappedMemoryRange, 1> memoryRanges;
	memoryRanges[0] = vk::MappedMemoryRange(_Memory, 0, VK_WHOLE_SIZE);
	_Device->GetDevice().flushMappedMemoryRanges(memoryRanges);

	_Device->GetDevice().unmapMemory(_Memory);
}

void Buffer::Transfer(const Buffer & dstBuffer, const vk::CommandPool &cmdPool)
{
	vk::CommandBuffer cmd = BeginSingleUseCommandBuffer(*_Device, cmdPool);

	vk::BufferCopy region(0, 0, _Size);
	cmd.copyBuffer(_Buffer, dstBuffer._Buffer, 1, &region);

	EndSingleUseCommandBuffer(cmd, *_Device, cmdPool);
}

void Buffer::Clean()
{
	if (_Memory) {
		_Device->GetDevice().freeMemory(_Memory);
	}

	if (_Buffer) {
		_Device->GetDevice().destroyBuffer(_Buffer);
	}
}
