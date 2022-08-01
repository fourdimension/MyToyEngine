#include "ContextManager.h"
#include "GpuResource.h"

void CommandContext::Transition(GpuResource& resource, D3D12_RESOURCE_STATES newState)
{
    // transition the vertex buffer data from copy destination state to vertex buffer state
    auto vResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetResource(), resource.m_usageState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_pCommandList->ResourceBarrier(1, &vResourceBarrier);
}
