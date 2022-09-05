#pragma once

#include "pch.h"
#include "GpuResource.h"

class ResourceUpload {


};

class UploadBuffer : public GpuResource
{
public:
	void Create(const std::wstring& name, size_t BufferSize);
	void UpdateData(ID3D12GraphicsCommandList* list, ID3D12Resource* buffer, void* pData);
protected:
	size_t m_bufferSize;
};

struct VertexPosTex
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT2 tex;
	static const D3D12_INPUT_ELEMENT_DESC inputLayout[2];
};

