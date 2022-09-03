#include "ContextManager.h"
#include "GpuResource.h"
#include "CommandListManager.h"
#include "d3dApp.h"

using namespace GameCore;

CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
    :m_type(type),
    m_pCmdListManager(nullptr),
    m_pCommandList(nullptr),
    m_pAllocator(nullptr),
    m_pGraphicsRootSignature(nullptr),
    m_pComputeRootSignature(nullptr),
    m_pPipelineState(nullptr)
{
    Initialize();
}

void CommandContext::Transition(GpuResource& resource, D3D12_RESOURCE_STATES newState)
{
    // transition the vertex buffer data from copy destination state to vertex buffer state
    auto vResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.GetResource(), resource.m_usageState, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
    m_pCommandList->ResourceBarrier(1, &vResourceBarrier);
}

void CommandContext::Initialize()
{
    ASSERT_SUCCEEDED(m_device != nullptr);
    m_pCmdListManager = new CommandListManager(m_device);
    m_pCmdListManager->CreateCommandList(m_type, m_pAllocator.Get(), m_pCommandList.Get());
}
