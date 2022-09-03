#include "GpuBuffer.h"
#include "d3dApp.h"
#include "DXSampleHelper.h"

using namespace GameCore;

void GpuBuffer::Create(const std::wstring& name, uint32_t numElements, uint32_t elementSize, const void* data)
{
    m_numElements = numElements;
    m_elementSize = elementSize;

    m_usageState = D3D12_RESOURCE_STATE_COPY_DEST;

    CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(data));
    CD3DX12_HEAP_PROPERTIES bufferProp(D3D12_HEAP_TYPE_DEFAULT);
    ThrowIfFailed(m_device->CreateCommittedResource(
        &bufferProp,
        D3D12_HEAP_FLAG_NONE,
        &desc,
        m_usageState,// we will start this heap in the copy destination state since we will copy data
                                        // from the upload heap to this heap
        nullptr,
        IID_PPV_ARGS(&m_pResource)));

    m_GpuVirtualAddress = m_pResource->GetGPUVirtualAddress();

    m_pResource->SetName(name.c_str());

    // initialize data
    if (data) 
    { }
}

