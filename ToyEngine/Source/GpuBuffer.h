#pragma once

#include "pch.h"
#include "GpuResource.h"

class GpuBuffer : public GpuResource {
public:
	GpuBuffer() : m_numElements(0), m_elementSize(0), m_bufferSize(0) {}

	D3D12_VERTEX_BUFFER_VIEW VertexBufferView(uint32_t strideSize, uint32_t bufferSize) const;
	D3D12_INDEX_BUFFER_VIEW IndexBufferView(uint32_t bufferSize) const;

	void Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* data = nullptr);

protected:
	uint32_t m_numElements;
	uint32_t m_elementSize;
	uint32_t m_bufferSize;
	
};

inline D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView( uint32_t strideSize,
	uint32_t bufferSize) const {

	D3D12_VERTEX_BUFFER_VIEW vertexView;
	vertexView.BufferLocation = m_GpuVirtualAddress;
	vertexView.StrideInBytes = strideSize;
	vertexView.SizeInBytes = bufferSize;
	return vertexView;
}

inline D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(uint32_t bufferSize) const {

	D3D12_INDEX_BUFFER_VIEW indexView;
	indexView.BufferLocation = m_GpuVirtualAddress;
	indexView.Format = DXGI_FORMAT_R32_UINT; // 32-bit unsigned integer (this is what a dword is, double word, a word is 2 bytes)
	indexView.SizeInBytes = bufferSize;
	return indexView;
}
