#include "ResourceUpload.h"
#include "d3dApp.h"

using namespace GameCore;

const D3D12_INPUT_ELEMENT_DESC VertexPosTex::inputLayout[] = {
     { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
     { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
};

void UploadBuffer::Create(const std::wstring& name, size_t BufferSize)
{
    m_bufferSize = BufferSize;
    // create upload heap
    // upload heaps are used to upload data to the GPU. CPU can write to it, GPU can read from it
    // We will upload the vertex buffer using this heap to the default heap

    CD3DX12_HEAP_PROPERTIES bufferHeapProp(D3D12_HEAP_TYPE_UPLOAD);
    auto desc = CD3DX12_RESOURCE_DESC::Buffer(BufferSize);
    m_device->CreateCommittedResource(
        &bufferHeapProp, // upload heap
        D3D12_HEAP_FLAG_NONE, // no flags
        &desc, // resource description for a buffer
        D3D12_RESOURCE_STATE_GENERIC_READ, // GPU will read from this buffer and copy its contents to the default heap
        nullptr,
        IID_PPV_ARGS(&m_pResource));

    m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();
    m_pResource->SetName(name.c_str());
}

void UploadBuffer::UpdateData(ID3D12GraphicsCommandList* list, ID3D12Resource* buffer, void* pData)
{
    // store  ivertex buffern upload heap
    D3D12_SUBRESOURCE_DATA resourceData = {};
    resourceData.pData = reinterpret_cast<BYTE*>(pData); // pointer to our vertex array
    resourceData.RowPitch = m_bufferSize; // size of all our triangle vertex data
    resourceData.SlicePitch = m_bufferSize; // also the size of our triangle vertex data

    UpdateSubresources(list, buffer, m_pResource.Get(), 0, 0, 1, &resourceData);
}
