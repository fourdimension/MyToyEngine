/*     Copyright 2015-2017 Egor Yusov
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT OF ANY PROPRIETARY RIGHTS.
 *
 *  In no event and under no legal theory, whether in tort (including negligence),
 *  contract, or otherwise, unless required by applicable law (such as deliberate
 *  and grossly negligent acts) or agreed to in writing, shall any Contributor be
 *  liable for any damages, including any direct, indirect, special, incidental,
 *  or consequential damages of any character arising as a result of this License or
 *  out of the use or inability to use the software (including but not limited to damages
 *  for loss of goodwill, work stoppage, computer failure or malfunction, or any and
 *  all other commercial damages or losses), even if such Contributor has been advised
 *  of the possibility of such damages.
 */


#pragma once
#include "pch.h"
#include "RingAllocator.h"

// Constant blocks must be multiples of 16 constants @ 16 bytes each
#define DEFAULT_ALIGN 256



struct DynamicAllocation
{
	DynamicAllocation(ID3D12Resource *pBuff, size_t _offset, size_t _size) :
		pBuffer(pBuff),
		offset(_offset),
		size(_size)
	{}

	ID3D12Resource* pBuffer = nullptr;
	size_t offset = 0;
	size_t size = 0;
	void* CPUAddress = 0;
	D3D12_GPU_VIRTUAL_ADDRESS GPUAddress = 0;
};

class GPURingBuffer : public RingBuffer
{
public:
	GPURingBuffer(size_t MaxSize, bool AllowCPUAccess);

	GPURingBuffer(GPURingBuffer&& rhs) noexcept:
		RingBuffer(std::move(rhs)),
		m_CpuVirtualAddress(rhs.m_CpuVirtualAddress),
		m_GpuVirtualAddress(rhs.m_GpuVirtualAddress),
		m_pBuffer(std::move(rhs.m_pBuffer))
	{
		rhs.m_CpuVirtualAddress = nullptr;
		rhs.m_GpuVirtualAddress = 0;
		rhs.m_pBuffer = nullptr;
	}

	GPURingBuffer& operator = (GPURingBuffer&& rhs) noexcept	
	{
		if (this != &rhs) 
		{
			Free();
			RingBuffer::operator=(std::move(rhs));
			m_CpuVirtualAddress = rhs.m_CpuVirtualAddress;
			m_GpuVirtualAddress = rhs.m_GpuVirtualAddress;
			m_pBuffer = std::move(rhs.m_pBuffer);
			rhs.m_CpuVirtualAddress = nullptr;
			rhs.m_GpuVirtualAddress = 0;
		}

		return *this;
	}

	~GPURingBuffer();

	DynamicAllocation Allocate(size_t sizeInBytes);

	GPURingBuffer(const GPURingBuffer&) = delete;
	GPURingBuffer& operator =(GPURingBuffer&) = delete;

private:
	void Free();

	void* m_CpuVirtualAddress;
	D3D12_GPU_VIRTUAL_ADDRESS m_GpuVirtualAddress;
	Microsoft::WRL::ComPtr<ID3D12Resource> m_pBuffer;
};

class DynamicUploadHeap
{
public:

	DynamicUploadHeap() = default;
	void Initialize(bool bIsCPUAccessible, size_t InitialSize);

	//DynamicUploadHeap(const DynamicUploadHeap&) = delete;
	//DynamicUploadHeap(DynamicUploadHeap&&) = delete;
	//DynamicUploadHeap& operator=(const DynamicUploadHeap&) = delete;
	//DynamicUploadHeap& operator=(DynamicUploadHeap&&) = delete;

	DynamicAllocation Allocate(size_t SizeInBytes, size_t Alignment = DEFAULT_ALIGN);

	void FinishFrame(uint64_t FenceValue, uint64_t LastCompletedFenceValue);

private:
	bool m_IsCPUAccessible;
	// When a chunk of dynamic memory is requested, the heap first tries to allocate the memory in the largest GPU buffer. 
	// If allocation fails, a new ring buffer is created that provides enough space and requests memory from that buffer.
	// Only the largest buffer is used for allocation and all other buffers are released when GPU is done with corresponding frames
	std::vector<GPURingBuffer> m_RingBuffers;
	//std::mutex m_Mutex;
};

DynamicUploadHeap* GetUploadHeap();